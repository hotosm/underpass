resource "aws_secretsmanager_secret" "quay_robot_credentials" {
  name = "quay-robot-pull-credentials"

  tags = {
    name = "quay_robot_pull_credentials"
    Role = "Container Image access credentials"
  }
}

resource "aws_secretsmanager_secret_version" "quay_robot_credentials" {
  secret_id     = aws_secretsmanager_secret.quay_robot_credentials.id
  secret_string = jsonencode(var.quay_robot_credentials)
}

resource "aws_ecs_cluster" "galaxy" {
  name = "galaxy"

  capacity_providers = ["FARGATE", "FARGATE_SPOT"]
  setting {
    name  = "containerInsights"
    value = "enabled"
  }
  tags = {
    Name    = "galaxy"
    Role    = "galaxy"
    Project = "galaxy"
  }
}

/**
 * CloudWatch Logging access
 * Secrets Manager access
 * Container Registry access
**/
resource "aws_iam_role" "ecs_execution_role" {
  name_prefix = "galaxy-api-exec"
  path        = "/galaxy/"

  assume_role_policy = data.aws_iam_policy_document.ecs-assume-role.json

  inline_policy {
    name   = "galaxy-api-policy"
    policy = data.aws_iam_policy_document.galaxy-api-execution-role.json
  }

}

data "aws_kms_alias" "secretsmanager" {
  name = "alias/aws/secretsmanager"
}

data "aws_iam_policy_document" "ecs-assume-role" {
  statement {
    actions = ["sts:AssumeRole"]

    principals {
      type        = "Service"
      identifiers = ["ecs-tasks.amazonaws.com"]
    }

  }
}

data "aws_iam_policy_document" "galaxy-api-execution-role" {
  statement {
    sid = "1"

    actions = [
      "logs:CreateLogStream",
      "logs:PutLogEvents",
    ]

    resources = [
      "*",
    ]

  }

  statement {
    sid = "2"

    actions = [
      "secretsmanager:GetSecretValue"
    ]

    resources = [
      aws_secretsmanager_secret.underpass_database_credentials.arn,
      aws_secretsmanager_secret.quay_robot_credentials.arn,
    ]

  }

  statement {
    sid = "3"

    actions = [
      "kms:Decrypt"
    ]

    resources = [
      data.aws_kms_alias.secretsmanager.arn
    ]
  }
}

resource "aws_ecs_task_definition" "galaxy-api" {
  family                   = "galaxy-api"
  requires_compatibilities = ["FARGATE"]
  network_mode             = "awsvpc"
  cpu                      = 256
  memory                   = 512

  runtime_platform {
    operating_system_family = "LINUX"
    cpu_architecture        = "X86_64" // or ARM64
  }

  container_definitions = jsonencode([
    {
      name  = "galaxy-api"
      image = "quay.io/hotosm/galaxy-api:web-flow-a55d89f"
      repositoryCredentials = {
        credentialsParameter = aws_secretsmanager_secret.quay_robot_credentials.arn,
      }
      cpu       = 10
      memory    = 512
      essential = true
      portMappings = [
        {
          containerPort = 8080
          hostPort      = 8080
        }
      ]
      secrets = [
        { name : "POSTGRES_CONNECTION_PARAMS", valueFrom : aws_secretsmanager_secret_version.underpass_database_credentials.arn }
      ]
      environment = [

      ]
    }
  ])
  execution_role_arn = aws_iam_role.ecs_execution_role.arn
}

resource "aws_ecs_service" "galaxy-api" {
  name            = "galaxy-api"
  cluster         = aws_ecs_cluster.galaxy.id
  launch_type     = "FARGATE"
  task_definition = aws_ecs_task_definition.galaxy-api.arn
  desired_count   = 3

  propagate_tags = "SERVICE"

  network_configuration {
    subnets         = aws_subnet.public[*].id
    security_groups = [aws_security_group.api.id]
    // assign_public_ip = true // valid only for FARGATE
  }

  lifecycle {
    ignore_changes = [desired_count]
  }

}

resource "aws_lb" "osm-stats" {
  name               = "osm-stats"
  internal           = false
  load_balancer_type = "application"
  security_groups    = [aws_security_group.api.id]
  subnets            = aws_subnet.public[*].id

  enable_deletion_protection = false

  tags = {
    Environment = "production"
  }
}

resource "aws_lb_target_group" "osm-stats" {
  name     = "osm-stats"
  port     = 80
  protocol = "HTTP"
  vpc_id   = aws_vpc.underpass.id
}

resource "aws_lb_listener" "osm-stats-secure" {
  load_balancer_arn = aws_lb.osm-stats.arn
  port              = "443"
  protocol          = "HTTPS"
  ssl_policy        = "ELBSecurityPolicy-FS-1-2-Res-2019-08"
  certificate_arn   = data.aws_acm_certificate.wildcard.arn

  default_action {
    type             = "forward"
    target_group_arn = aws_lb_target_group.osm-stats.arn
  }
}

data "aws_acm_certificate" "wildcard" {
  domain      = "hotosm.org"
  statuses    = ["ISSUED"]
  most_recent = true
}

/** TODO: Change to include all instances
**/
resource "aws_lb_target_group_attachment" "underpass-instance" {
  target_group_arn = aws_lb_target_group.osm-stats.arn
  target_id        = aws_instance.file-processor[0].id
  port             = 80
}

