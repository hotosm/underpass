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

variable "database_instance_type" {
  type    = string
  default = "db.t3.micro"
}

variable "database_engine_version" {
  type    = string
  default = "12.7"
}

variable "database_name" {
  type    = string
  default = "underpass"
}

variable "database_username" {
  type    = string
  default = "mineworker"
}

variable "database_storage_min_capacity" {
  type    = number
  default = 100
}

variable "database_storage_max_capacity" {
  type    = number
  default = 1000
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

// Memory optimized with at least 8GB like r5a.large
variable "file_processor_instance_type" {
  type    = string
  default = "t3.micro"
}

variable "file_processor_count" {
  type    = number
  default = 1
}

variable "file_processor_ebs_size" {
  type    = number
  default = 1000
}

variable "api_server_count" {
  type    = number
  default = 0
}

variable "api_server_ebs_size" {
  type    = number
  default = 100
}

variable "api_server_instance_type" {
  type    = string
  default = "t3.micro"
}

variable "debfile_name" {
  type    = string
  default = "underpass_20210614_amd64.deb"
}

variable "libpqxx_version" {
  type    = string
  default = "7.6.1"
}

