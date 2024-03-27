data "aws_availability_zones" "available" {
  state = "available"
}

// attribute - main_route_table_id
data "aws_vpc" "tasking-manager" {
  id = var.tasking-manager_vpc_id
}

module "vpc" {
  source = "git::https://gitlab.com/eternaltyro/terraform-aws-vpc.git"

  project_meta = {
    name       = "underpass"
    short_name = "underpass"
    version    = "stable"
    url        = "underpass.hotosm.org"
  }

  deployment_environment = var.deployment_environment

  default_tags = {
    project        = "underpass"
    maintainer     = "dk"
    documentation  = "docs.underpass.org" /* update */
    cost_center    = null
    IaC_Management = "Terraform"
  }
}

resource "aws_instance" "file-processor" {
  ami           = data.aws_ami.ubuntu_latest_arm.id
  instance_type = lookup(var.instance_types, "file_processor", "r6g.xlarge")

  subnet_id              = module.vpc.private_subnets[0]
  vpc_security_group_ids = [module.vpc.default_security_group_id]

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
    Name        = "underpass-processor-${count.index}"
    Description = "Changefile processor server"
    Project     = "Data_Access"
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
