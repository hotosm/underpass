variable "aws_region" {
  type    = string
  default = "us-east-1"
}

variable "vpc_cidr" {
  type    = string
  default = "10.0.0.0/16"
}

// Memory optimized with at least 8GB
variable "app_instance_type" {
  type    = string
  default = "r5a.large"
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
  type = string
}

variable "underpass_database_credentials" {
  type = map(string)

  default = {
    engine               = "postgres"
    port                 = 5432
    username             = "underpass"
    password             = ""
    dbinstanceidentifier = ""
    host                 = ""
    dbname               = ""
  }
}

variable "debfile_name" {
  type    = string
  default = "underpass_20210614_amd64.deb"
}

variable "libpqxx_version" {
  type    = string
  default = "7.3.1"
}
