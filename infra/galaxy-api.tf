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

  ephemeral_storage {
    size_in_gib = 200
  }

  execution_role_arn = aws_iam_role.ecs_execution_role.arn

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

}

resource "aws_ecs_service" "galaxy-api" {
  name            = "api"
  cluster         = aws_ecs_cluster.galaxy.id
  launch_type     = "FARGATE"
  task_definition = aws_ecs_task_definition.galaxy-api.arn
  desired_count   = 1

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

resource "aws_appautoscaling_target" "galaxy-api" {
  max_capacity       = 5
  min_capacity       = 1
  resource_id        = "service/${aws_ecs_cluster.galaxy.name}/${aws_ecs_service.galaxy-api.name}"
  scalable_dimension = "ecs:service:DesiredCount"
  service_namespace  = "ecs"
}

resource "aws_appautoscaling_policy" "galaxy-api-memory" {
  name               = "scale-by-memory-load"
  policy_type        = "TargetTrackingScaling"
  resource_id        = aws_appautoscaling_target.galaxy-api.resource_id
  scalable_dimension = aws_appautoscaling_target.galaxy-api.scalable_dimension
  service_namespace  = aws_appautoscaling_target.galaxy-api.service_namespace

  target_tracking_scaling_policy_configuration {
    scale_in_cooldown  = 60
    scale_out_cooldown = 60

    predefined_metric_specification {
      predefined_metric_type = "ECSServiceAverageMemoryUtilization"
    }

    target_value = "75"
  }
}

resource "aws_appautoscaling_policy" "galaxy-api-cpu" {
  name               = "scale-by-cpu-load"
  policy_type        = "TargetTrackingScaling"
  resource_id        = aws_appautoscaling_target.galaxy-api.resource_id
  scalable_dimension = aws_appautoscaling_target.galaxy-api.scalable_dimension
  service_namespace  = aws_appautoscaling_target.galaxy-api.service_namespace

  target_tracking_scaling_policy_configuration {
    scale_in_cooldown  = 60
    scale_out_cooldown = 40

    predefined_metric_specification {
      predefined_metric_type = "ECSServiceAverageCPUUtilization"
    }

    target_value = "75"
  }
}

data "aws_arn" "api-alb" {
  arn = aws_lb.galaxy-api.id
}

data "aws_arn" "api-targetgroup" {
  arn = aws_lb_target_group.galaxy-api.id
}

resource "aws_appautoscaling_policy" "galaxy-api-alb-requests" {
  name               = "scale-by-requests-to-alb"
  policy_type        = "TargetTrackingScaling"
  resource_id        = aws_appautoscaling_target.galaxy-api.resource_id
  scalable_dimension = aws_appautoscaling_target.galaxy-api.scalable_dimension
  service_namespace  = aws_appautoscaling_target.galaxy-api.service_namespace

  target_tracking_scaling_policy_configuration {
    scale_in_cooldown  = 120
    scale_out_cooldown = 120

    predefined_metric_specification {
      predefined_metric_type = "ALBRequestCountPerTarget"
      resource_label         = join("/", [trimprefix(data.aws_arn.api-alb.resource, "loadbalancer/"), data.aws_arn.api-targetgroup.resource])
    }

    target_value = "100"
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
  ssl_policy        = var.alb_tls_policy
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
  ip_address_type            = "dualstack"

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
  ssl_policy        = var.alb_tls_policy
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
  secret_string = base64encode(
    templatefile(
      "${path.module}/config.txt.tftpl",
      {
        insights_creds        = var.insights_db_credentials
        underpass_creds       = var.underpass_db_credentials
        tasking_manager_creds = var.tasking_manager_db_credentials

        dump_path = var.dump_path

        oauth2_creds = var.oauth2_credentials

        api_url  = "${var.api_url_scheme}${var.api_host}"
        api_port = var.api_port

        sentry_dsn = var.sentry_dsn
      }
    )
  )
}

