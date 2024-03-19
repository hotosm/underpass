
## Do we want to replace this with eternaltyro aurora rds?

resource "random_password" "underpass_database_admin_password" {
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

resource "aws_secretsmanager_secret" "underpass_database_credentials" {
  name = "${var.deployment_environment}/underpass/database"

  description = "Underpass database password and connection string params"

  tags = {
    name = "Underpass Database Admin Credentials"
    Role = "Database access credentials"
  }
}

resource "aws_secretsmanager_secret_version" "underpass_database_credentials" {
  secret_id = aws_secretsmanager_secret.underpass_database_credentials.id
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
      aws_db_instance.underpass.id,
      aws_db_instance.underpass.db_name,
      aws_db_instance.underpass.engine,
      aws_db_instance.underpass.address,
      aws_db_instance.underpass.port,
      aws_db_instance.underpass.username,
      random_password.underpass_database_admin_password.result,
    ]
  ))
}

data "aws_iam_policy_document" "access-underpass-database-credentials" {

  statement {
    sid = "1"

    actions = [
      "secretsmanager:GetSecretValue"
    ]

    resources = [
      aws_secretsmanager_secret.underpass_database_credentials.arn,
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

resource "aws_iam_role" "access-underpass-database-credentials" {
  name_prefix = "underpass-db-cred-access"
  path        = "/underpass/"

  assume_role_policy = data.aws_iam_policy_document.rds-proxy-assume-role.json

  inline_policy {
    name   = "access-database-credentials"
    policy = data.aws_iam_policy_document.access-underpass-database-credentials.json
  }

}

resource "aws_db_subnet_group" "underpass" {
  name       = "underpass"
  subnet_ids = module.vpc.private_subnets
}

resource "aws_security_group" "database" {
  name        = "database"
  description = "Underpass Database"
  vpc_id      = module.vpc.vpc_id

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
    security_groups = [module.vpc.default_security_group_id]
  }

  egress {
    from_port        = 0
    to_port          = 0
    protocol         = "-1"
    cidr_blocks      = ["0.0.0.0/0"]
    ipv6_cidr_blocks = ["::/0"]
  }

  tags = {
    Name = "underpass-database"
  }

}

resource "aws_security_group" "database-administration" {
  name        = "database-administration"
  description = "Ephemeral user access to underpass Database"
  vpc_id      = module.vpc.vpc_id

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
    Name = "underpass-database"
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
resource "aws_db_instance" "underpass" {
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
        "underpass",
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
  password = random_password.underpass_database_admin_password.result

  skip_final_snapshot       = true
  final_snapshot_identifier = var.database_final_snapshot_identifier

  iam_database_authentication_enabled = true

  vpc_security_group_ids = [
    aws_security_group.database.id,
    aws_security_group.database-administration.id
  ]
  db_subnet_group_name = aws_db_subnet_group.underpass.name

  tags = {
    Name = "underpass-db"
    Role = "Database server"
  }

}

resource "aws_db_proxy" "underpass" {
  name                = "underpass"
  engine_family       = "POSTGRESQL"
  idle_client_timeout = 1800
  require_tls         = true
  role_arn            = aws_iam_role.access-underpass-database-credentials.arn
  vpc_security_group_ids = [
    aws_security_group.database.id,
    aws_security_group.database-administration.id
  ]
  vpc_subnet_ids = module.vpc.private_subnets

  auth {
    auth_scheme = "SECRETS"
    description = "underpass database admin credentials"
    iam_auth    = "DISABLED"
    secret_arn  = aws_secretsmanager_secret.underpass_database_credentials.arn
  }
}

resource "aws_db_proxy_default_target_group" "underpass" {
  db_proxy_name = aws_db_proxy.underpass.name

  connection_pool_config {
    connection_borrow_timeout = 120
    # init_query                   = "SET x=1, y=2"
    max_connections_percent      = 100
    max_idle_connections_percent = 50
    session_pinning_filters      = ["EXCLUDE_VARIABLE_SETS"]
  }
}

resource "aws_db_proxy_target" "underpass" {
  db_instance_identifier = aws_db_instance.underpass.id
  db_proxy_name          = aws_db_proxy.underpass.name
  target_group_name      = aws_db_proxy_default_target_group.underpass.name
}

resource "aws_db_proxy_endpoint" "underpass-readonly" {
  db_proxy_name          = aws_db_proxy.underpass.name
  db_proxy_endpoint_name = "underpass-readonly"
  vpc_subnet_ids         = module.vpc.private_subnets
  vpc_security_group_ids = [
    aws_security_group.database.id,
    aws_security_group.database-administration.id
  ]
  target_role = "READ_ONLY"
}
