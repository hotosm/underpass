data "aws_availability_zones" "available" {
  state = "available"
}

resource "aws_vpc" "underpass" {
  cidr_block = var.vpc_cidr

  tags = {
    Name       = "underpass"
    Maintainer = "Yogesh Girikumar"
    Terraform  = "true"
  }
}

resource "aws_subnet" "public" {
  count             = var.subnet_count
  vpc_id            = aws_vpc.underpass.id
  availability_zone = data.aws_availability_zones.available.names[count.index]

  cidr_block = cidrsubnet(var.vpc_cidr, 8, "${count.index + 1}")

  tags = {
    Name = "underpass-public${count.index + 1}"
  }

}

resource "aws_subnet" "private" {
  count             = var.subnet_count
  vpc_id            = aws_vpc.underpass.id
  availability_zone = data.aws_availability_zones.available.names[count.index]

  cidr_block = cidrsubnet(var.vpc_cidr, 8, "${count.index + var.subnet_count + 1}")

  tags = {
    Name = "underpass-private${count.index + 1}"
  }

}


# ec2 instances etc should explicitly depend on this
resource "aws_internet_gateway" "internet" {
  vpc_id = aws_vpc.underpass.id

  tags = {
    Name = "main"
  }
}

resource "aws_route_table" "public" {
  vpc_id = aws_vpc.underpass.id

  route {
    cidr_block = "0.0.0.0/0"
    gateway_id = aws_internet_gateway.internet.id
  }

  tags = {
    Name = "underpass-public"
  }
}

resource "aws_eip" "nat" {
  vpc = true
}

resource "aws_nat_gateway" "nat" {
  allocation_id = aws_eip.nat.id
  subnet_id     = aws_subnet.public[1].id
}

resource "aws_route" "private" {
  route_table_id         = aws_vpc.underpass.default_route_table_id
  destination_cidr_block = "0.0.0.0/0"
  nat_gateway_id         = aws_nat_gateway.nat.id
  depends_on             = [aws_vpc.underpass]
}

// EXPLICIT ASSOCIATIONS
resource "aws_route_table_association" "private" {
  count          = var.subnet_count
  subnet_id      = aws_subnet.private[count.index].id
  route_table_id = aws_vpc.underpass.default_route_table_id
}

resource "aws_route_table_association" "public" {
  count          = var.subnet_count
  subnet_id      = aws_subnet.public[count.index].id
  route_table_id = aws_route_table.public.id
}

resource "aws_security_group" "database" {
  name        = "database"
  description = "Underpass Database"
  vpc_id      = aws_vpc.underpass.id

  ingress {
    description     = "Allow connection to database from API"
    from_port       = 5432
    to_port         = 5432
    protocol        = "tcp"
    security_groups = [aws_security_group.api.id]
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

resource "aws_security_group" "app" {
  name        = "app"
  description = "Underpass Application Firewall"
  vpc_id      = aws_vpc.underpass.id

  ingress {
    description      = "All TLS connections"
    from_port        = 443
    to_port          = 443
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
    Name = "underpass-app"
  }

}

resource "aws_security_group" "api" {
  name        = "api"
  description = "Underpass Backend API Firewall"
  vpc_id      = aws_vpc.underpass.id

  ingress {
    description      = "Allow self to access"
    from_port        = 443
    to_port          = 443
    protocol         = "tcp"
    cidr_blocks      = ["0.0.0.0/0"]
    ipv6_cidr_blocks = ["::/0"]
    #    security_groups  = [aws_security_group.app.id]
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

