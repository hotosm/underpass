variable "aws_region" {
  type    = string
  default = "us-east-1"
}

variable "vpc_cidr" {
  type    = string
  default = "10.0.0.0/16"
}

variable "app_instance_type" {
  type    = string
  default = "t3.micro"
}

variable "api_instance_type" {
  type    = string
  default = "t3.micro"
}

variable "db_instance_type" {
  type    = string
  default = "db.t3.micro"
}

variable "subnet_count" {
  type    = number
  default = 4
}

variable "ssh_key_pair_name" {
  type    = string
}
