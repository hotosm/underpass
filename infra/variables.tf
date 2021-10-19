variable "deployment_environment" {
  type = map(string)

  default = {
    demo       = "demo"
    staging    = "staging"
    production = ""
  }
}

variable "instance_types" {
  type = map(string)

  default = {
    api_server     = "t3.micro"
    file_processor = "r6g.xlarge"
    database       = "db.t4g.micro"
  }
}

variable "aws_region" {
  type    = string
  default = "us-east-1"
}

variable "vpc_cidr" {
  type    = string
  default = "10.0.0.0/16"
}

variable "subnet_count" {
  type    = number
  default = 4
}

variable "ssh_key_pair_name" {
  type = string
}

variable "database_engine_version" {
  type    = string
  default = "12.8"
}

variable "database_name" {
  type    = string
  default = "underpass"
}

variable "database_username" {
  type    = string
  default = "mineworker"
}

variable "database_final_snapshot_identifier" {
  type    = string
  default = "bye-underpass"
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

variable "server_count" {
  type = map(number)

  default = {
    file_processor = 1
    api_server     = 1
  }
}

variable "disk_sizes" {
  type = map(number)

  default = {
    file_processor_root  = 36
    file_processor_extra = 2000
    api_server_root      = 10
    api_server_extra     = 100
    db_min               = 100
    db_max               = 1000
  }
}

variable "debfile_name" {
  type    = string
  default = "underpass_20210614_amd64.deb"
}

variable "libpqxx_version" {
  type    = string
  default = "7.6.1"
}
