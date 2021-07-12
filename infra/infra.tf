# CloudFront 
# ACM Cert
# Cloudwatch monitoring
# Cloudwatch Alarms
# Load Balancer?
# Secrets Manager entry

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

data "aws_ami" "ubuntu" {
  most_recent = true

  filter {
    name   = "name"
    values = ["ubuntu/images/hvm-ssd/ubuntu-focal-20.04-amd64-server-*"]
  }

  filter {
    name   = "virtualization-type"
    values = ["hvm"]
  }

  filter {
    name   = "architecture"
    values = ["x86_64"] # or arm64
  }

  owners = ["099720109477"] # Canonical
}

data "aws_ami" "debian" {
  most_recent = true

  filter {
    name   = "name"
    values = ["debian-10-amd64-*"]
  }

  filter {
    name   = "virtualization-type"
    values = ["hvm"]
  }

  filter {
    name   = "architecture"
    values = ["x86_64"] # or arm64
  }

  owners = ["136693071363"] # Debian
}

data "aws_ami" "debian_bullseye" {
  most_recent = true

  filter {
    name   = "name"
    values = ["debian-11-amd64-*"]
  }

  filter {
    name   = "virtualization-type"
    values = ["hvm"]
  }

  filter {
    name   = "architecture"
    values = ["x86_64"] # or arm64
  }

  owners = ["903794441882"] # Debian
}

resource "aws_launch_configuration" "processor_conf" {
  name_prefix   = "underpass-changeset-processor-"
  image_id      = data.aws_ami.ubuntu.id
  instance_type = var.app_instance_type

  iam_instance_profile        = aws_iam_instance_profile.underpass.name
  key_name                    = var.ssh_key_pair_name
  security_groups             = [aws_security_group.app.id]
  associate_public_ip_address = false # TODO: Review

  root_block_device {
    volume_size = 10
  }

  ebs_block_device {
    device_name = "/dev/sdb"
    volume_size = 100
    volume_type = "gp3"
    throughput  = 125
  }

}

resource "aws_autoscaling_group" "changefile_processor" {
  name_prefix          = "underpass-changeset-processor-"
  launch_configuration = aws_launch_configuration.processor_conf.name
  min_size             = 1
  max_size             = 2

  vpc_zone_identifier = aws_subnet.private[*].id

  lifecycle {
    create_before_destroy = true
  }
}

/** TODO
* Needs to access S3 bucket
* Needs to be behind public ALB?
* Put this in an auto-scaling group
*/
resource "aws_instance" "file-processor" {
  ami           = data.aws_ami.ubuntu.id
  instance_type = var.app_instance_type

  subnet_id              = aws_subnet.private[2].id
  vpc_security_group_ids = [aws_security_group.app.id]

  root_block_device {
    volume_size = 10
    volume_type = "gp3"
    throughput  = 125
  }

  ebs_block_device {
    device_name = "/dev/sdb"
    volume_size = 100
    volume_type = "gp3"
    throughput  = 125
  }

  iam_instance_profile = aws_iam_instance_profile.underpass.name
  key_name             = var.ssh_key_pair_name

  tags = {
    Name = "underpass-application"
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
  ami           = data.aws_ami.ubuntu.id
  instance_type = var.api_instance_type

  subnet_id              = aws_subnet.private[2].id
  vpc_security_group_ids = [aws_security_group.api.id]

  root_block_device {
    volume_size = 10
    volume_type = "gp3"
    throughput  = 125
  }

  ebs_block_device {
    device_name = "/dev/sdb"
    volume_size = 100
    volume_type = "gp3"
    throughput  = 125
  }

  iam_instance_profile = aws_iam_instance_profile.underpass.name

  tags = {
    Name = "underpass-api"
  }
}

resource "aws_db_instance" "underpass" {
  allocated_storage         = 100
  max_allocated_storage     = 1000 # Storage auto-scaling
  engine                    = "postgres"
  engine_version            = "12.7"
  instance_class            = var.db_instance_type
  name                      = "underpass"
  username                  = "mineworker"
  password                  = "shovelANDhammer"
  skip_final_snapshot       = true
  final_snapshot_identifier = "bye-underpass"
}

data "aws_route53_zone" "hotosm-org" {
  name = "hotosm.org."
}

/**
resource "aws_route53_record" "underpass" {
  zone_id = data.aws_route53_zone.hotosm-org.zone_id
  name    = "underpass.${data.aws_route53_zone.hotosm-org.name}"
  type    = "A"
  ttl     = "300"
  records = [aws_eip.underpass.public_ip]
}
*/

/** TODO
* Figure out Lifecycle rules
* Figure out ACL and perms
*
resource "aws_s3_bucket" "underpass" {
  bucket_prefix = "underpass"
  acl           = "private"

  tags = {
    name = "something"
  }

}
*/

