# CloudFront 
# ACM Cert
# Cloudwatch monitoring
# Cloudwatch Alarms
# Load Balancer?

resource "aws_iam_instance_profile" "underpass" {
  name = "underpass_instance_profile"
  role = aws_iam_role.underpass.name
}

data "aws_iam_policy_document" "assume-role-ec2" {
  statement {
    actions = ["sts:AssumeRole"]

    principals {
      type        = "Service"
      identifiers = ["ec2.amazonaws.com"]
    }

  }
}

data "aws_iam_policy_document" "instance-connect" {
  statement {
    actions = ["ec2-instance-connect:SendSSHPublicKey"]
    resources = [
      "arn:aws:ec2:region:account-id:instance/i-1234567890abcdef0",
      "arn:aws:ec2:region:account-id:instance/i-0598c7d356eba48d7"
    ]
  }
}

data "aws_iam_policy" "metrics" {
  name = "CloudWatchAgentServerPolicy"
}

data "aws_iam_policy" "SSM" {
  name = "AmazonSSMManagedInstanceCore"
}

/** TODO
Figure out interpolation
- Name of bucket - interpolation using bucket resource
- Conditional restriction for NAT gateway public IP
**/
data "aws_iam_policy_document" "underpass" {
  statement {
    sid = "1"

    actions = [
      "s3:ListAllMyBuckets",
    ]

    resources = [
      "*",
    ]

  }

  statement {
    sid = "2"

    actions = [
      "s3:PutObject",
      "s3:GetObject",
      "s3:ListBucket",
      "s3:DeleteObject",
    ]

    resources = [
      "arn:aws:s3:::underpass*",
      "arn:aws:s3:::underpass/*",
    ]

  }
}

resource "aws_iam_role" "underpass" {
  name = "underpass_role"
  path = "/"

  assume_role_policy = data.aws_iam_policy_document.assume-role-ec2.json

  inline_policy {
    name   = "underpass-policy"
    policy = data.aws_iam_policy_document.underpass.json
  }

  managed_policy_arns = [
    data.aws_iam_policy.metrics.arn,
    data.aws_iam_policy.SSM.arn,
  ]
}

/** TODO
* Monitoring:
*   - Setup Newrelic Agent
*   - Setup CloudWatchAgent
* User Access (SSH Ingress):
* App Access (Ports Ingress):
*   - Port: ????
* Service Access (IAM Egress):
*   - Needs to access S3 bucket
* Apps installed:
*   - postgresql-client
*   - netcat
*/
resource "aws_instance" "file-processor" {
  ami           = data.aws_ami.ubuntu_latest_arm.id
  instance_type = lookup(var.instance_types, "file_processor", "r6g.xlarge")

  subnet_id              = aws_subnet.private[2].id
  vpc_security_group_ids = [aws_security_group.app.id]

  root_block_device {
    volume_size = lookup(var.disk_sizes, "file_processor_root", 10)
    volume_type = "gp3"
    throughput  = 125
  }

  ebs_block_device {
    device_name = "/dev/sdb"
    volume_size = lookup(var.disk_sizes, "file_processor_extra", 1000)
    volume_type = "gp3"
    throughput  = 125
  }

  iam_instance_profile = aws_iam_instance_profile.underpass.name
  key_name             = var.ssh_key_pair_name

  tags = {
    Name = "underpass-processor-${count.index}"
    Role = "Changefile processor server"
  }

  // Install everything
  /**
  user_data = templatefile(
    "${path.module}/bootstrap.tpl",
    {
      debfile_name    = var.debfile_name,
      libpqxx_version = var.libpqxx_version
    }
  )
  */

  count = lookup(var.server_count, "file_processor", 1)

  lifecycle {
    create_before_destroy = false
    prevent_destroy       = false
    ignore_changes = [
      # Ignore changes to AMI
      ami,
    ]
  }
}

/*
resource "aws_eip" "underpass" {
  instance = aws_instance.application.id
  vpc      = true
}
*/

/** TODO
* Monitoring:
*   - Setup Newrelic Agent
*   - Setup CloudWatchAgent
* User Access (SSH Ingress):
*   - Add SSH key
* App Access (Ports Ingress):
*   - Port: ????
*   - Needs to be behind private ALB?
* Service Access (IAM Egress):
*   - Needs to access S3 bucket
*   - Needs to access Database
*   - Needs to access Secrets manager entry for DB creds?
*/
resource "aws_instance" "api" {
  ami           = data.aws_ami.ubuntu_latest.id
  instance_type = lookup(var.instance_types, "api_server", "t3.micro")

  subnet_id              = aws_subnet.private[2].id
  vpc_security_group_ids = [aws_security_group.api.id]

  root_block_device {
    volume_size = lookup(var.disk_sizes, "api_server_root", 10)
    volume_type = "gp3"
    throughput  = 125
  }

  ebs_block_device {
    device_name = "/dev/sdb"
    volume_size = lookup(var.disk_sizes, "api_server_extra", 100)
    volume_type = "gp3"
    throughput  = 125
  }

  iam_instance_profile = aws_iam_instance_profile.underpass.name
  key_name             = var.ssh_key_pair_name

  tags = {
    Name = "underpass-api"
    Role = "API Server"
  }

  count = lookup(var.server_count, "api_server", 1)

  lifecycle {
    create_before_destroy = false
    prevent_destroy       = false
    ignore_changes = [
      # Ignore changes to AMI
      ami,
    ]
  }
}

resource "random_password" "underpass_database_password_string" {
  length  = 32
  special = false
  number  = true
  lower   = true
  upper   = true

  keepers = {
    # Generate new id each time we switch to a new ami_id
    ## ami_id = var.ami_id
  }
}

resource "aws_secretsmanager_secret" "underpass_database_credentials" {
  name = "underpass-db"

  tags = {
    name = "underpass"
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
      aws_db_instance.underpass.name,
      aws_db_instance.underpass.engine,
      aws_db_instance.underpass.address,
      aws_db_instance.underpass.port,
      aws_db_instance.underpass.username,
      random_password.underpass_database_password_string.result,
    ]
  ))
}

resource "aws_db_subnet_group" "galaxy" {
  name       = "galaxy"
  subnet_ids = aws_subnet.private[*].id
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
  identifier = trim(join("-", ["underpass", lookup(var.deployment_environment, "production", "0")]), "-")

  allocated_storage     = lookup(var.disk_sizes, "db_min", 100)
  max_allocated_storage = lookup(var.disk_sizes, "db_max", 1000) # Storage auto-scaling

  engine         = "postgres"
  engine_version = var.database_engine_version
  instance_class = lookup(var.instance_types, "database", "db.t4g.micro")

  name     = var.database_name
  username = var.database_username
  password = random_password.underpass_database_password_string.result

  skip_final_snapshot       = true
  final_snapshot_identifier = var.database_final_snapshot_identifier

  iam_database_authentication_enabled = true

  vpc_security_group_ids = [aws_security_group.database.id]
  db_subnet_group_name   = aws_db_subnet_group.galaxy.name

  tags = {
    Name = "underpass"
    Role = "Database server"
  }
}

data "aws_route53_zone" "hotosm-org" {
  name = "hotosm.org."
}

/**
resource "aws_route53_record" "underpass" {
  zone_id = data.aws_route53_zone.hotosm-org.zone_id
  name    = "underpass-demo.${data.aws_route53_zone.hotosm-org.name}"
  type    = "A"
  ttl     = "300"
  records = [aws_eip.underpass.public_ip]
}
*/

/** TODO
* Figure out Lifecycle rules
* Figure out ACL and perms
*
*/
resource "aws_s3_bucket" "underpass" {
  bucket_prefix = "underpass"
  acl           = "private"

  tags = {
    name = "underpass"
    Role = "Backup store"
  }

}

