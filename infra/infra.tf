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
* Needs to access S3 bucket
* Add CloudWatchAgent
*/
resource "aws_instance" "file-processor" {
  ami = data.aws_ami.ubuntu-latest.id
  // ami           = var.file_processor_ami
  instance_type = var.file_processor_instance_type

  subnet_id              = aws_subnet.private[2].id
  vpc_security_group_ids = [aws_security_group.app.id]

  root_block_device {
    volume_size = 10
    volume_type = "gp3"
    throughput  = 125
  }

  ebs_block_device {
    device_name = "/dev/sdb"
    volume_size = var.file_processor_ebs_size
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
  user_data = templatefile(
    "${path.module}/bootstrap.tpl",
    {
      debfile_name    = var.debfile_name,
      libpqxx_version = var.libpqxx_version
    }
  )

  count = var.file_processor_count

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
* Needs to access Database
* Needs to be behind private ALB?
* Needs to access Secrets manager entry for DB creds?
* Add SSH key
*/
resource "aws_instance" "api" {
  ami           = data.aws_ami.ubuntu-latest.id
  instance_type = var.api_server_instance_type

  subnet_id              = aws_subnet.private[2].id
  vpc_security_group_ids = [aws_security_group.api.id]

  root_block_device {
    volume_size = 10
    volume_type = "gp3"
    throughput  = 125
  }

  ebs_block_device {
    device_name = "/dev/sdb"
    volume_size = var.api_server_ebs_size
    volume_type = "gp3"
    throughput  = 125
  }

  iam_instance_profile = aws_iam_instance_profile.underpass.name
  key_name             = var.ssh_key_pair_name

  tags = {
    Name = "underpass-api"
    Role = "API Server"
  }

  count = var.api_server_count

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

resource "aws_db_instance" "underpass" {
  allocated_storage         = var.database_storage_min_capacity
  max_allocated_storage     = var.database_storage_max_capacity # Storage auto-scaling
  engine                    = "postgres"
  engine_version            = var.database_engine_version
  instance_class            = var.database_instance_type
  name                      = var.database_name
  username                  = var.database_username
  password                  = random_password.underpass_database_password_string.result
  skip_final_snapshot       = true
  final_snapshot_identifier = var.database_final_snapshot_identifier

  tags = {
    name = "underpass"
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

