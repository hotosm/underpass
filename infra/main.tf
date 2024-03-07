data "aws_availability_zones" "available" {
  state = "available"
}

// attribute - main_route_table_id
data "aws_vpc" "tasking-manager" {
  id = var.tasking-manager_vpc_id
}

module "vpc" {
  source = "git::https://gitlab.com/eternaltyro/terraform-aws-vpc.git"
  # vpc_id = var.vpc_id
  # cidr_block = var.cidr_block

  project_meta = {
    name = "underpass"
    short_name = "underpass"
    version = "stable"
    url = "underpass.hotosm.org"
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


resource "aws_vpc_peering_connection" "underpass-tasking-manager" {
  tags = {
    Name = "underpass-taskingmanager"
  }
  vpc_id      = module.vpc.vpc_id
  peer_vpc_id = data.aws_vpc.tasking-manager.id
  auto_accept = true

  accepter {
    allow_remote_vpc_dns_resolution = true
  }

  requester {
    allow_remote_vpc_dns_resolution = true
  }
}

/********************************************
** THIS BLOCK EDITS TASKING MANAGER VPC
**               WHICH IS
** OUTSIDE THE SCOPE OF UNDERPASS
*********************************************/
resource "aws_route" "tasking-manager-vpc-to-peering-connection" {
  route_table_id            = data.aws_vpc.tasking-manager.main_route_table_id
  destination_cidr_block    = module.vpc.cidr_block
  vpc_peering_connection_id = aws_vpc_peering_connection.underpass-tasking-manager.id
  depends_on                = [module.vpc, aws_vpc_peering_connection.underpass-tasking-manager]

}

resource "aws_security_group" "app" {
  name        = "app"
  description = "Underpass Application Firewall"
  vpc_id      = module.vpc.vpc_id

  ingress {
    description      = "All TLS connections"
    from_port        = 443
    to_port          = 443
    protocol         = "tcp"
    cidr_blocks      = ["0.0.0.0/0"]
    ipv6_cidr_blocks = ["::/0"]
  }

  /** TODO:
  To protect this instance public IP needs to be removed
  **/
  ingress {
    description      = "Allow access to cloudfront"
    from_port        = 80
    to_port          = 80
    protocol         = "tcp"
    cidr_blocks      = ["0.0.0.0/0"]
    ipv6_cidr_blocks = ["::/0"]
  }

  ingress {
    description     = "Allow access to Bastion"
    from_port       = 22
    to_port         = 22
    protocol        = "tcp"
    security_groups = [aws_security_group.remote-access.id]
  }

  egress {
    from_port        = 0
    to_port          = 0
    protocol         = "-1"
    cidr_blocks      = ["0.0.0.0/0"]
    ipv6_cidr_blocks = ["::/0"]
  }

  tags = {
    Name = "underpass-app"
  }

}

resource "aws_security_group" "remote-access" {
  name        = "remote-access"
  description = "Underpass Backend API Firewall"
  vpc_id      = module.vpc.vpc_id

  ingress {
    description = "Allow SSH from nobody do not remove"
    from_port   = 22
    to_port     = 22
    protocol    = "tcp"
    cidr_blocks = ["127.0.0.1/32"]
  }

  egress {
    from_port        = 0
    to_port          = 0
    protocol         = "-1"
    cidr_blocks      = ["0.0.0.0/0"]
    ipv6_cidr_blocks = ["::/0"]
  }

  tags = {
    Name = "underpass-remote-user-access"
  }

  lifecycle {
    ignore_changes = [
      ingress
    ]
  }

}

resource "aws_security_group" "api" {
  name        = "api"
  description = "Underpass Backend API Firewall"
  vpc_id      = module.vpc.vpc_id

  ingress {
    description      = "Allow HTTPS from World"
    from_port        = 443
    to_port          = 443
    protocol         = "tcp"
    cidr_blocks      = ["0.0.0.0/0"]
    ipv6_cidr_blocks = ["::/0"]
  }

  /** TODO:
  To protect this instance public IP needs to be removed
**/
  ingress {
    description      = "Allow HTTP from World"
    from_port        = 80
    to_port          = 80
    protocol         = "tcp"
    cidr_blocks      = ["0.0.0.0/0"]
    ipv6_cidr_blocks = ["::/0"]
  }

  /** API containers and the load-balancer use the same security group
   and tcp/8000 need only be allowed from self.
  **/
  ingress {
    description = "Allow access to API App"
    from_port   = 8000
    to_port     = 8000
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
    Name = "underpass-api"
  }

}

resource "aws_security_group" "vpc-endpoint" {
  name        = "vpc-endpoint"
  description = "Firewall for to VPC endpoints from containers and instances"
  vpc_id      = module.vpc.vpc_id

  ingress {
    description      = "Allow secure access from interfaces in VPC"
    from_port        = 443
    to_port          = 443
    protocol         = "tcp"
    cidr_blocks      = [module.vpc.cidr_block]
    ipv6_cidr_blocks = ["::/0"]
  }

  egress {
    from_port        = 0
    to_port          = 0
    protocol         = "-1"
    cidr_blocks      = ["0.0.0.0/0"]
    ipv6_cidr_blocks = ["::/0"]
  }

  tags = {
    Name = "underpass-vpc-endpoint"
  }

}

resource "aws_vpc_endpoint" "secretsmanager" {
  vpc_id = module.vpc.vpc_id

  vpc_endpoint_type = "Interface"
  service_name      = "com.amazonaws.us-east-1.secretsmanager" // TODO: use var.aws_region

  private_dns_enabled = true
  auto_accept         = true

  subnet_ids = [for subnet in aws_subnet.public : subnet.id]

  security_group_ids = [aws_security_group.vpc-endpoint.id]
}

resource "aws_vpc_endpoint" "awslogs" {
  vpc_id = module.vpc.vpc_id

  vpc_endpoint_type = "Interface"
  service_name      = "com.amazonaws.us-east-1.logs" // TODO: use var.aws_region

  private_dns_enabled = true
  auto_accept         = true

  subnet_ids = [for subnet in aws_subnet.public : subnet.id]

  security_group_ids = [aws_security_group.vpc-endpoint.id]
}
