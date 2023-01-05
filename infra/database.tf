
resource "random_password" "galaxy_database_admin_password" {
  length  = 32
  special = false
  numeric = true
  lower   = true
  upper   = true

  keepers = {
    # Generate new id each time we switch to a new ami_id
    ## ami_id = var.ami_id
  }
}

resource "aws_secretsmanager_secret" "galaxy_database_credentials" {
  name = "${var.deployment_environment}/galaxy/database"

  description = "Galaxy database password and connection string params"

  tags = {
    name = "Galaxy Database Admin Credentials"
    Role = "Database access credentials"
  }
}

resource "aws_secretsmanager_secret_version" "galaxy_database_credentials" {
  secret_id = aws_secretsmanager_secret.galaxy_database_credentials.id
  secret_string = jsonencode(zipmap(
    [
      "dbinstanceidentifier",
      "dbname",
      "engine",
      "host",
      "port",
      "username",
      "password",
    ],
    [
      aws_db_instance.galaxy.id,
      aws_db_instance.galaxy.db_name,
      aws_db_instance.galaxy.engine,
      aws_db_instance.galaxy.address,
      aws_db_instance.galaxy.port,
      aws_db_instance.galaxy.username,
      random_password.galaxy_database_admin_password.result,
    ]
  ))
}

data "aws_iam_policy_document" "access-galaxy-database-credentials" {

  statement {
    sid = "1"

    actions = [
      "secretsmanager:GetSecretValue"
    ]

    resources = [
      aws_secretsmanager_secret.galaxy_database_credentials.arn,
    ]

  }

  statement {
    sid = "2"

    actions = [
      "kms:Decrypt"
    ]

    resources = [
      data.aws_kms_alias.secretsmanager.arn
    ]
  }
}

data "aws_iam_policy_document" "rds-proxy-assume-role" {
  statement {
    actions = ["sts:AssumeRole"]

    principals {
      type        = "Service"
      identifiers = ["rds.amazonaws.com"]
    }

  }
}

resource "aws_iam_role" "access-galaxy-database-credentials" {
  name_prefix = "galaxy-db-cred-access"
  path        = "/galaxy/"

  assume_role_policy = data.aws_iam_policy_document.rds-proxy-assume-role.json

  inline_policy {
    name   = "access-database-credentials"
    policy = data.aws_iam_policy_document.access-galaxy-database-credentials.json
  }

}

resource "aws_db_subnet_group" "galaxy" {
  name       = "galaxy"
  subnet_ids = [for subnet in aws_subnet.private : subnet.id]
}

resource "aws_security_group" "database" {
  name        = "database"
  description = "Underpass Database"
  vpc_id      = aws_vpc.galaxy.id

  ingress {
    description = "Allow from self"
    from_port   = 5432
    to_port     = 5432
    protocol    = "tcp"
    self        = true
  }

  ingress {
    description     = "Allow from App and API"
    from_port       = 5432
    to_port         = 5432
    protocol        = "tcp"
    security_groups = [aws_security_group.api.id, aws_security_group.app.id]
  }

  egress {
    from_port        = 0
    to_port          = 0
    protocol         = "-1"
    cidr_blocks      = ["0.0.0.0/0"]
    ipv6_cidr_blocks = ["::/0"]
  }

  tags = {
    Name = "galaxy-database"
  }

}

resource "aws_security_group" "database-administration" {
  name        = "database-administration"
  description = "Ephemeral user access to Galaxy Database"
  vpc_id      = aws_vpc.galaxy.id

  revoke_rules_on_delete = true

  lifecycle {
    ignore_changes = [
      ingress,
    ]

    create_before_destroy = true
  }

  ingress {
    description = "Allow from self"
    from_port   = 5432
    to_port     = 5432
    protocol    = "tcp"
    self        = true
  }

  egress {
    from_port        = 0
    to_port          = 0
    protocol         = "-1"
    cidr_blocks      = ["0.0.0.0/0"]
    ipv6_cidr_blocks = ["::/0"]
  }

  tags = {
    Name = "galaxy-database"
  }

}

/** TODO
* Monitoring:
*   - Configure enhanced monitoring
*   - Setup CloudWatch Alerts
* User Access (Ingress):
*   - hot_readonly and hot_readwrite roles
*   - Enable IAM auth
* App Access (tcp/5432 Ingress):
*   - IP: file_processor.ip?
* Backup:
*   - Backup and snapshot expiry
*/
resource "aws_db_instance" "galaxy" {
  lifecycle {
    ignore_changes = [
      engine_version,
      allocated_storage,
    ]
  }

  identifier = trim(
    join(
      "-",
      [
        "galaxy",
        lookup(var.name_suffix, var.deployment_environment, "0")
      ]
    ),
    "-"
  )

  allocated_storage = lookup(var.disk_sizes, "db_min", 100)

  # Storage auto-scaling atleast 1.1x allocated storage
  max_allocated_storage = lookup(var.disk_sizes, "db_max", 1000)

  engine         = "postgres"
  engine_version = var.database_engine_version
  instance_class = lookup(var.instance_types, "database", "db.t4g.micro")

  db_name  = var.database_name
  username = var.database_username
  password = random_password.galaxy_database_admin_password.result

  skip_final_snapshot       = true
  final_snapshot_identifier = var.database_final_snapshot_identifier

  iam_database_authentication_enabled = true

  vpc_security_group_ids = [
    aws_security_group.database.id,
    aws_security_group.database-administration.id
  ]
  db_subnet_group_name = aws_db_subnet_group.galaxy.name

  tags = {
    Name = "galaxy-db"
    Role = "Database server"
  }

}

resource "aws_db_proxy" "galaxy" {
  name                = "galaxy"
  engine_family       = "POSTGRESQL"
  idle_client_timeout = 1800
  require_tls         = true
  role_arn            = aws_iam_role.access-galaxy-database-credentials.arn
  vpc_security_group_ids = [
    aws_security_group.database.id,
    aws_security_group.database-administration.id
  ]
  vpc_subnet_ids = [for subnet in aws_subnet.private : subnet.id]

  auth {
    auth_scheme = "SECRETS"
    description = "Galaxy database admin credentials"
    iam_auth    = "DISABLED"
    secret_arn  = aws_secretsmanager_secret.galaxy_database_credentials.arn
  }
}

resource "aws_db_proxy_default_target_group" "galaxy" {
  db_proxy_name = aws_db_proxy.galaxy.name

  connection_pool_config {
    connection_borrow_timeout = 120
    # init_query                   = "SET x=1, y=2"
    max_connections_percent      = 100
    max_idle_connections_percent = 50
    session_pinning_filters      = ["EXCLUDE_VARIABLE_SETS"]
  }
}

resource "aws_db_proxy_target" "galaxy" {
  db_instance_identifier = aws_db_instance.galaxy.id
  db_proxy_name          = aws_db_proxy.galaxy.name
  target_group_name      = aws_db_proxy_default_target_group.galaxy.name
}

resource "aws_db_proxy_endpoint" "galaxy-readonly" {
  db_proxy_name          = aws_db_proxy.galaxy.name
  db_proxy_endpoint_name = "galaxy-readonly"
  vpc_subnet_ids         = [for subnet in aws_subnet.private : subnet.id]
  vpc_security_group_ids = [
    aws_security_group.database.id,
    aws_security_group.database-administration.id
  ]
  target_role = "READ_ONLY"
}
