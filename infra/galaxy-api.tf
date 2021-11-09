resource "aws_ecs_cluster" "galaxy" {
  name = "galaxy"

  capacity_providers = ["FARGATE", "FARGATE_SPOT"]
  setting {
    name  = "containerInsights"
    value = "enabled"
  }
  tags = {
    Name = ""
    Role = ""
  }
}

data "aws_iam_role" "ecs_execution_role" {
  name = "ecsTaskExecutionRole"
}

resource "aws_ecs_task_definition" "galaxy-api" {
  family = "galaxy-api"
  container_definitions = jsonencode([
    {
      name      = "galaxy-api"
      image     = "service-first"
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
        { name : "POSTGRES_CONNECT_PARAM_JSON", valueFrom : aws_secretsmanager_secret_version.underpass_database_credentials.arn }
      ]
    }
  ])
  execution_role_arn = data.aws_iam_role.ecs_execution_role.arn
}

resource "aws_ecs_service" "galaxy-api" {
  name            = "galaxy-api"
  cluster         = aws_ecs_cluster.galaxy.id
  task_definition = aws_ecs_task_definition.galaxy-api.arn
  desired_count   = 3
  //  iam_role        = aws_iam_role.foo.arn
  //  depends_on      = [aws_iam_role_policy.foo]

  ordered_placement_strategy {
    type  = "binpack"
    field = "cpu"
  }

  lifecycle {
    ignore_changes = [desired_count]
  }

}

