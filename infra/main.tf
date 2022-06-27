data "aws_availability_zones" "available" {
  state = "available"
}

// attribute - main_route_table_id
data "aws_vpc" "tasking-manager" {
  id = var.tasking-manager_vpc_id
}

resource "aws_vpc" "galaxy" {
  cidr_block           = var.vpc_cidr
  enable_dns_hostnames = true

  tags = {
    Name       = "Galaxy"
    Maintainer = "Yogesh Girikumar"
    Terraform  = "true"
  }
}

resource "aws_subnet" "public" {
  count             = var.subnet_count
  vpc_id            = aws_vpc.galaxy.id
  availability_zone = data.aws_availability_zones.available.names[count.index]

  cidr_block = cidrsubnet(var.vpc_cidr, 8, "${count.index + 1}")

  tags = {
    Name = "galaxy-public${count.index + 1}"
  }

}

resource "aws_subnet" "private" {
  count             = var.subnet_count
  vpc_id            = aws_vpc.galaxy.id
  availability_zone = data.aws_availability_zones.available.names[count.index]

  cidr_block = cidrsubnet(var.vpc_cidr, 8, "${count.index + var.subnet_count + 1}")

  tags = {
    Name = "galaxy-private${count.index + 1}"
  }

}

# ec2 instances etc should explicitly depend on this
resource "aws_internet_gateway" "internet" {
  vpc_id = aws_vpc.galaxy.id

  tags = {
    Name = "main"
  }
}

resource "aws_eip" "nat" {
  vpc = true

  tags = {
    Name = "Galaxy NAT Gateway"
  }
}

resource "aws_nat_gateway" "nat" {
  allocation_id = aws_eip.nat.id
  subnet_id     = aws_subnet.public[1].id

  tags = {
    Name = "Galaxy"
  }
}

resource "aws_vpc_peering_connection" "galaxy-tasking-manager" {
  tags = {
    Name = "galaxy-taskingmanager"
  }
  vpc_id      = aws_vpc.galaxy.id
  peer_vpc_id = data.aws_vpc.tasking-manager.id
  auto_accept = true

  accepter {
    allow_remote_vpc_dns_resolution = true
  }

  requester {
    allow_remote_vpc_dns_resolution = true
  }
}

resource "aws_route_table" "public" {
  vpc_id = aws_vpc.galaxy.id

  route {
    cidr_block = "0.0.0.0/0"
    gateway_id = aws_internet_gateway.internet.id
  }

  route {
    cidr_block                = data.aws_vpc.tasking-manager.cidr_block
    vpc_peering_connection_id = aws_vpc_peering_connection.galaxy-tasking-manager.id
  }

  tags = {
    Name = "galaxy-public"
  }
}

resource "aws_route_table" "private" {
  vpc_id = aws_vpc.galaxy.id

  route {
    cidr_block = "0.0.0.0/0"
    gateway_id = aws_nat_gateway.nat.id
  }

  route {
    cidr_block                = data.aws_vpc.tasking-manager.cidr_block
    vpc_peering_connection_id = aws_vpc_peering_connection.galaxy-tasking-manager.id
  }

  tags = {
    Name = "galaxy-private"
  }
}

// EXPLICIT ASSOCIATIONS
resource "aws_route_table_association" "private" {
  count          = var.subnet_count
  subnet_id      = aws_subnet.private[count.index].id
  route_table_id = aws_route_table.private.id
}

resource "aws_route_table_association" "public" {
  count          = var.subnet_count
  subnet_id      = aws_subnet.public[count.index].id
  route_table_id = aws_route_table.public.id
}

/********************************************
** THIS BLOCK EDITS TASKING MANAGER VPC
**               WHICH IS
** OUTSIDE THE SCOPE OF GALAXY / UNDERPASS
*********************************************/
resource "aws_route" "tasking-manager-vpc-to-peering-connection" {
  route_table_id            = data.aws_vpc.tasking-manager.main_route_table_id
  destination_cidr_block    = aws_vpc.galaxy.cidr_block
  vpc_peering_connection_id = aws_vpc_peering_connection.galaxy-tasking-manager.id
  depends_on                = [aws_vpc.galaxy, aws_vpc_peering_connection.galaxy-tasking-manager]

}

resource "aws_security_group" "database" {
  name        = "database"
  description = "Underpass Database"
  vpc_id      = aws_vpc.galaxy.id

  ingress {
    description     = "Allow connection to database from API"
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

resource "aws_security_group" "app" {
  name        = "app"
  description = "Underpass Application Firewall"
  vpc_id      = aws_vpc.galaxy.id

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

  egress {
    from_port        = 0
    to_port          = 0
    protocol         = "-1"
    cidr_blocks      = ["0.0.0.0/0"]
    ipv6_cidr_blocks = ["::/0"]
  }

  tags = {
    Name = "galaxy-app"
  }

}

resource "aws_security_group" "api" {
  name        = "api"
  description = "Underpass Backend API Firewall"
  vpc_id      = aws_vpc.galaxy.id

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

  ingress {
    description      = "Allow access to API App"
    from_port        = 8000
    to_port          = 8000
    protocol         = "tcp"
    cidr_blocks      = ["0.0.0.0/0"]
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
    Name = "galaxy-api"
  }

}

resource "aws_security_group" "vpc-endpoint" {
  name        = "vpc-endpoint"
  description = "Firewall for to VPC endpoints from containers and instances"
  vpc_id      = aws_vpc.galaxy.id

  ingress {
    description      = "Allow secure access from interfaces in VPC"
    from_port        = 443
    to_port          = 443
    protocol         = "tcp"
    cidr_blocks      = [aws_vpc.galaxy.cidr_block]
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
    Name = "galaxy-vpc-endpoint"
  }

}

resource "aws_vpc_endpoint" "secretsmanager" {
  vpc_id = aws_vpc.galaxy.id

  vpc_endpoint_type = "Interface"
  service_name      = "com.amazonaws.us-east-1.secretsmanager" // TODO: use var.aws_region

  private_dns_enabled = true
  auto_accept         = true

  //  subnet_ids = concat(aws_subnet.private[*].id, aws_subnet.public[*].id)
  subnet_ids = aws_subnet.public[*].id

  security_group_ids = [aws_security_group.vpc-endpoint.id]
}

resource "aws_vpc_endpoint" "awslogs" {
  vpc_id = aws_vpc.galaxy.id

  vpc_endpoint_type = "Interface"
  service_name      = "com.amazonaws.us-east-1.logs" // TODO: use var.aws_region

  private_dns_enabled = true
  auto_accept         = true

  subnet_ids = aws_subnet.public[*].id

  security_group_ids = [aws_security_group.vpc-endpoint.id]
}
