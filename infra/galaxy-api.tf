resource "aws_cloudwatch_log_group" "galaxy" {
  name              = var.project_name
  retention_in_days = 7
}

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
  name = var.project_name

  setting {
    name  = "containerInsights"
    value = "enabled"
  }

  tags = {
    Name    = var.project_name
    Role    = "galaxy"
    Project = var.project_name
  }
}

resource "aws_ecs_cluster_capacity_providers" "galaxy" {
  cluster_name = aws_ecs_cluster.galaxy.name

  capacity_providers = ["FARGATE", "FARGATE_SPOT"]

  default_capacity_provider_strategy {
    base              = 1
    weight            = 100
    capacity_provider = "FARGATE"
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
      "logs:CreateLogGroup",
      "logs:CreateLogStream",
      "logs:PutLogEvents",
    ]

    resources = [ // TODO: Improve?
      "arn:aws:logs:*:670261699094:log-group:galaxy",
      "arn:aws:logs:*:670261699094:log-group:galaxy:log-stream:*",
    ]

  }

  statement {
    sid = "2"

    actions = [
      "secretsmanager:GetSecretValue"
    ]

    resources = [
      aws_secretsmanager_secret.galaxy_database_credentials.arn,
      aws_secretsmanager_secret.quay_robot_credentials.arn,
      aws_secretsmanager_secret.configfile.arn,
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
      image = var.container_image_uri

      repositoryCredentials = {
        credentialsParameter = aws_secretsmanager_secret.quay_robot_credentials.arn
      }

      cpu       = 10
      memory    = 512
      essential = true

      portMappings = [
        {
          containerPort = 8000
          hostPort      = 8000
        },
      ]

      secrets = [
        {
          name      = "POSTGRES_CONNECTION_PARAMS"
          valueFrom = aws_secretsmanager_secret_version.galaxy_database_credentials.arn
        },
        {
          name      = "GALAXY_API_CONFIG_FILE"
          valueFrom = aws_secretsmanager_secret_version.configfile.arn
        },
      ]

      logConfiguration = {
        logDriver = "awslogs"
        options = {
          awslogs-group         = aws_cloudwatch_log_group.galaxy.name
          awslogs-region        = var.aws_region
          awslogs-stream-prefix = "api"
        }
      }

      environment = [

      ]
    }
  ])
  execution_role_arn = aws_iam_role.ecs_execution_role.arn
}

resource "aws_ecs_service" "galaxy-api" {
  name            = "api"
  cluster         = aws_ecs_cluster.galaxy.id
  launch_type     = "FARGATE"
  task_definition = aws_ecs_task_definition.galaxy-api.arn
  desired_count   = 3

  propagate_tags = "SERVICE"

  network_configuration {
    subnets          = aws_subnet.public[*].id
    security_groups  = [aws_security_group.api.id]
    assign_public_ip = true // valid only for FARGATE
  }

  load_balancer {
    target_group_arn = aws_lb_target_group.galaxy-api.arn
    container_name   = "galaxy-api"
    container_port   = 8000
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
  vpc_id   = aws_vpc.galaxy.id
  health_check {
    enabled = true
    path    = "/"
  }
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

resource "aws_lb" "galaxy-api" {
  name               = "galaxy-api"
  internal           = false
  load_balancer_type = "application"
  security_groups    = [aws_security_group.api.id]
  subnets            = aws_subnet.public[*].id

  enable_deletion_protection = false

  tags = {
    Environment = "production"
  }
}

resource "aws_lb_target_group" "galaxy-api" {
  name        = "galaxy-api"
  port        = 8000
  protocol    = "HTTP"
  vpc_id      = aws_vpc.galaxy.id
  target_type = "ip"

  health_check {
    enabled = true
    path    = "/docs"
  }
}

resource "aws_lb_listener" "galaxy-api-secure" {
  load_balancer_arn = aws_lb.galaxy-api.arn
  port              = "443"
  protocol          = "HTTPS"
  ssl_policy        = "ELBSecurityPolicy-FS-1-2-Res-2019-08"
  certificate_arn   = data.aws_acm_certificate.wildcard.arn

  default_action {
    type             = "forward"
    target_group_arn = aws_lb_target_group.galaxy-api.arn
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

resource "aws_secretsmanager_secret" "configfile" {
  name = "${var.deployment_environment}/galaxy/api-configfile"
}

resource "aws_secretsmanager_secret_version" "configfile" {
  secret_id = aws_secretsmanager_secret.configfile.id
  secret_string = base64gzip(
    templatefile(
      "${path.module}/config.txt.tpl",
      {
        insights_pg_host     = lookup(var.insights_db_config_credentials, "host", "")
        insights_pg_port     = lookup(var.insights_db_config_credentials, "port", "")
        insights_pg_user     = lookup(var.insights_db_config_credentials, "user", "")
        insights_pg_password = lookup(var.insights_db_config_credentials, "password", "")
        insights_pg_database = lookup(var.insights_db_config_credentials, "database", "")

        underpass_pg_host     = lookup(var.underpass_db_config_credentials, "host", "")
        underpass_pg_port     = lookup(var.underpass_db_config_credentials, "port", "")
        underpass_pg_user     = lookup(var.underpass_db_config_credentials, "user", "")
        underpass_pg_password = lookup(var.underpass_db_config_credentials, "password", "")
        underpass_pg_database = lookup(var.underpass_db_config_credentials, "database", "")

        tasking_manager_pg_host     = lookup(var.tasking_manager_db_config_credentials, "host", "")
        tasking_manager_pg_port     = lookup(var.tasking_manager_db_config_credentials, "port", "")
        tasking_manager_pg_user     = lookup(var.tasking_manager_db_config_credentials, "user", "")
        tasking_manager_pg_password = lookup(var.tasking_manager_db_config_credentials, "password", "")
        tasking_manager_pg_database = lookup(var.tasking_manager_db_config_credentials, "database", "")

        dump_path = var.dump_path

        oauth2_client_id     = lookup(var.oauth2_credentials, "client_id", "")
        oauth2_client_secret = lookup(var.oauth2_credentials, "client_secret", "")
        oauth2_secret_key    = lookup(var.oauth2_credentials, "secret_key", "")
      }
    )
  )
}

