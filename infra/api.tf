resource "aws_cloudwatch_log_group" "underpass" {
  name              = "/${var.deployment_environment}/${var.project_name}"
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

resource "aws_ecs_cluster" "underpass" {
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

resource "aws_iam_instance_profile" "underpass" {
  name = "underpass_instance_profile"
  role = aws_iam_role.underpass.name
}

resource "aws_iam_role" "underpass" {
  name_prefix = "underpass-api-"
  path        = "/underpass/"

  assume_role_policy = data.aws_iam_policy_document.ecs-assume-role.json
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

data "aws_iam_policy_document" "underpass-api-execution-role" {
  statement {
    sid = "1"

    actions = [
      "logs:CreateLogGroup",
      "logs:CreateLogStream",
      "logs:PutLogEvents",
    ]

    resources = [
      aws_cloudwatch_log_group.underpass.arn,
      "${aws_cloudwatch_log_group.underpass.arn}:log-stream:*",
    ]

  }

  statement {
    sid = "2"

    actions = [
      "secretsmanager:GetSecretValue"
    ]

    resources = [
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

data "aws_kms_alias" "secretsmanager" {
  name = "alias/aws/secretsmanager"
}

module "ecs" {
  source       = "git::https://gitlab.com/eternaltyro/terraform-aws-ecs.git"
  service_name = "underpass-${var.deployment_environment}"
  default_tags = {
    project       = "Data_Access"
    maintainer    = "DK"
    documentation = "https://docs.hotosm.org/"
    cost_center   = ""
  }
  container_secrets = [
    {
      name      = "POSTGRES_CONNECTION_PARAMS"
      valueFrom = aws_secretsmanager_secret_version.underpass_database_credentials.arn
    },
    {
      name      = "GALAXY_API_CONFIG_FILE"
      valueFrom = aws_secretsmanager_secret_version.configfile.arn
  }]
  container_envvars = [
    {
      name  = "FORWARDED_ALLOW_IPS"
      value = "*"
    },
    {
      name  = "SENTRY_ENVIRONMENT"
      value = var.deployment_environment
    },
    {
      name  = "SENTRY_RELEASE"
      value = lookup(var.sentry, "app_release", "galaxy-api@vDEFAULT")
    }
  ]
  container_settings = {
    service_name = "underpass-${var.deployment_environment}"
    app_port     = 8000
    image_url    = "ghcr.io/hotosm/underpass"
    image_tag    = "ci"
  }
  container_cpu_architecture = "X86_64"
  # container_commands = [
  #   "",

  # ]
  container_capacity = {
    cpu       = 256
    memory_mb = 512
  }
  tasks_count = {
    desired_count   = 1
    min_healthy_pct = 100
    max_pct         = 200
  }
  log_configuration = {
    logdriver = "awslogs"
    options = {
      awslogs-group         = "underpass"
      awslogs-region        = "us-east-1"
      awslogs-stream-prefix = "underpass-"
    }
  }
  scaling_target_values = {
    cpu_pct             = 85
    memory_pct          = 85
    request_count       = 50
    container_min_count = 1
    container_max_count = 5
  }
  service_subnets          = module.vpc.public_subnets
  aws_vpc_id               = module.vpc.vpc_id
  service_security_groups  = [module.vpc.default_security_group_id]        // update for update
  alb_security_group       = module.alb.load_balancer_app_security_group //public or private
  ecs_cluster_arn          = aws_ecs_cluster.underpass.arn
  ecs_cluster_name         = aws_ecs_cluster.underpass.name
  task_role_arn            = aws_iam_role.underpass.arn
  load_balancer_enabled    = true
  load_balancer_arn_suffix = module.alb.load_balancer_arn_suffix
  target_group_arn_suffix  = module.alb.target_group_arn_suffix
  target_group_arn         = module.alb.target_group_arn
  efs_enabled              = false
  efs_settings = {
    file_system_id     = "string"
    root_directory     = "string"
    access_point_id    = "string"
    transit_encryption = "DISABLED"
    iam_authz          = "DISABLED"
  }
}

data "aws_acm_certificate" "wildcard" {
  domain      = "hotosm.org"
  statuses    = ["ISSUED"]
  most_recent = true
}

resource "aws_appautoscaling_target" "underpass-api" {
  max_capacity       = 5
  min_capacity       = 1
  resource_id        = "service/${aws_ecs_cluster.underpass.name}/${module.ecs.service_name}"
  scalable_dimension = "ecs:service:DesiredCount"
  service_namespace  = "ecs"
}

resource "aws_appautoscaling_policy" "underpass-api-alb-requests" {
  name               = "scale-by-requests-to-alb"
  policy_type        = "TargetTrackingScaling"
  resource_id        = aws_appautoscaling_target.underpass-api.resource_id
  scalable_dimension = aws_appautoscaling_target.underpass-api.scalable_dimension
  service_namespace  = aws_appautoscaling_target.underpass-api.service_namespace

  target_tracking_scaling_policy_configuration {
    scale_in_cooldown  = 120
    scale_out_cooldown = 30

    predefined_metric_specification {
      predefined_metric_type = "ALBRequestCountPerTarget"
      resource_label         = "app/${module.alb.load_balancer_arn_suffix}/targetgroup/${module.alb.target_group_arn_suffix}"
    }

    target_value = "5"
  }
}

module "alb" {
  source            = "git::https://gitlab.com/eternaltyro/terraform-aws-alb.git"
  alb_name          = "underpass-${var.deployment_environment}"
  target_group_name = "underpass-${var.deployment_environment}"
  default_tags = { // TODO: move to top level var for consistency across resources
    project       = "underpass"
    maintainer    = "DK"
    documentation = "https://docs.hotosm.org/"
    cost_center   = ""
  }
  alb_subnets         = module.vpc.public_subnets
  health_check_path   = "/v1/docs"
  acm_tls_cert_domain = data.aws_acm_certificate.wildcard.domain
  vpc_id              = module.vpc.vpc_id
  app_port            = 8000
  //ip_address_type
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
        underpass_db_creds = var.underpass_db_credentials

        oauth2_creds = var.oauth2_credentials

        api_url             = "${var.api_url_scheme}${var.api_host}"
        api_port            = var.api_port
        api_export_max_area = var.api_export_max_area
        api_log_level       = var.api_log_level

        sentry_dsn             = lookup(var.sentry, "dsn")
        sentry_app_environment = var.deployment_environment
        sentry_app_release_tag = "underpass-api@${var.container_image_tag}"
      }
    )
  )
}
