# ACM Cert
# Cloudwatch monitoring
# Cloudwatch Alarms
# Load Balancer?

data "aws_s3_bucket" "galaxy-website" {
  bucket = "galaxy-ui-website"
}

data "aws_acm_certificate" "hotosm-wildcard" {
  domain      = "hotosm.org"
  statuses    = ["ISSUED"]
  types       = ["AMAZON_ISSUED"]
  most_recent = true

}

data "aws_cloudfront_cache_policy" "galaxy" {
  name = "Managed-CachingOptimized"
}

data "aws_cloudfront_origin_request_policy" "galaxy" {
  name = "Managed-UserAgentRefererHeaders"
}

## TODO:
# Add Origin Access Policy

resource "aws_cloudfront_distribution" "galaxy" {
  aliases = ["galaxy1.hotosm.org"]
  comment = "Galaxy production CDN"

  enabled         = true
  is_ipv6_enabled = true

  origin {
    domain_name = data.aws_s3_bucket.galaxy-website.bucket_regional_domain_name
    origin_id   = "galaxy-website"
  }

  default_cache_behavior {
    allowed_methods        = ["HEAD", "GET"]
    cached_methods         = ["HEAD", "GET"]
    viewer_protocol_policy = "redirect-to-https"
    compress               = true
    target_origin_id       = "galaxy-website"

    cache_policy_id          = data.aws_cloudfront_cache_policy.galaxy.id
    origin_request_policy_id = data.aws_cloudfront_origin_request_policy.galaxy.id

  }

  viewer_certificate {
    acm_certificate_arn      = data.aws_acm_certificate.hotosm-wildcard.arn
    minimum_protocol_version = "TLSv1.2_2021"
    ssl_support_method       = "sni-only"
  }

  restrictions {
    geo_restriction {
      restriction_type = "none"
    }
  }

}

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

resource "aws_instance" "jumphost" {
  ami           = data.aws_ami.debian_bullseye.id
  instance_type = lookup(var.instance_types, "jump_host", "t3.large")

  subnet_id              = aws_subnet.public[2].id
  vpc_security_group_ids = [aws_security_group.remote-access.id]

  root_block_device {
    volume_size = lookup(var.disk_sizes, "jump_host_root", 10)
    volume_type = "gp3"
    throughput  = 125
  }

  iam_instance_profile = aws_iam_instance_profile.underpass.name
  key_name             = var.ssh_key_pair_name

  tags = {
    Name = "galaxy-jump.hotosm.org"
    Role = "SSH Jump Host"
  }

  lifecycle {
    create_before_destroy = false
    prevent_destroy       = false
    ignore_changes = [
      # Ignore changes to AMI
      # ami,
    ]
  }
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

resource "aws_instance" "file-processor-2" {
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
    Name = "underpass-processor"
    Role = "Changefile processor server"
  }

  lifecycle {
    create_before_destroy = false
    prevent_destroy       = false
    ignore_changes = [
      # Ignore changes to AMI
      ami,
    ]
  }
}

resource "aws_eip" "jumphost" {
  instance = aws_instance.jumphost.id
  vpc      = true
}

resource "random_password" "galaxy_database_admin_password" {
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

resource "aws_secretsmanager_secret" "galaxy_database_credentials" {
  name = "${var.deployment_environment}/galaxy/database"

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
      aws_db_instance.galaxy.name,
      aws_db_instance.galaxy.engine,
      aws_db_instance.galaxy.address,
      aws_db_instance.galaxy.port,
      aws_db_instance.galaxy.username,
      random_password.galaxy_database_admin_password.result,
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
resource "aws_db_instance" "galaxy" {
  lifecycle {
    ignore_changes = [
      engine_version,
      max_allocated_storage

      # Manually updated often
      // instance_class,
    ]
  }

  identifier = trim(join("-", ["galaxy", lookup(var.name_suffix, var.deployment_environment, "0")]), "-")

  allocated_storage     = lookup(var.disk_sizes, "db_min", 100)
  max_allocated_storage = lookup(var.disk_sizes, "db_max", 1000) # Storage auto-scaling

  engine         = "postgres"
  engine_version = var.database_engine_version
  instance_class = lookup(var.instance_types, "database", "db.t4g.micro")

  db_name  = var.database_name
  username = var.database_username
  password = random_password.galaxy_database_admin_password.result

  skip_final_snapshot       = true
  final_snapshot_identifier = var.database_final_snapshot_identifier

  iam_database_authentication_enabled = true

  vpc_security_group_ids = [aws_security_group.database.id]
  db_subnet_group_name   = aws_db_subnet_group.galaxy.name

  tags = {
    Name = "galaxy-db"
    Role = "Database server"
  }

}

data "aws_route53_zone" "hotosm-org" {
  name = "hotosm.org."
}

resource "aws_route53_record" "galaxy-api-lb" {
  zone_id = data.aws_route53_zone.hotosm-org.zone_id
  name    = "galaxy-api.${data.aws_route53_zone.hotosm-org.name}"
  type    = "A"

  alias {
    name                   = aws_lb.galaxy-api.dns_name
    zone_id                = aws_lb.galaxy-api.zone_id
    evaluate_target_health = true
  }
}

resource "aws_route53_record" "jumphost" {
  zone_id = data.aws_route53_zone.hotosm-org.zone_id
  name    = "galaxy-bastion.${data.aws_route53_zone.hotosm-org.name}"
  type    = "A"
  ttl     = "300"
  records = [aws_eip.jumphost.public_ip]
}

