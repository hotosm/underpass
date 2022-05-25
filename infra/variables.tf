variable "project_name" {
  type = string

  default = "galaxy"
}

/*****************************************
** SECRET VARIABES AND SENSITIVE STRINGS
******************************************/

variable "quay_robot_credentials" {
  type = map(string)

  default = {
    username = "quay"
    password = "DummyDefault123!"
  }

  sensitive = true
}

variable "tasking_manager_db_credentials" {
  type = map(string)

  default = {
    host     = "localhost"
    port     = "5432"
    database = "postgres"
    user     = "postgres_dba"
    password = "secret123"
  }

  sensitive = true
}

variable "underpass_db_credentials" {
  type = map(string)

  default = {
    host     = "localhost"
    port     = "5432"
    database = "postgres"
    user     = "postgres_dba"
    password = "secret123"
  }

  sensitive = true
}

variable "insights_db_credentials" {
  type = map(string)

  default = {
    host     = "localhost"
    port     = "5432"
    database = "postgres"
    user     = "postgres_dba"
    password = "secret123"
  }

  sensitive = true
}

variable "oauth2_credentials" {
  type = map(string)

  default = {
    client_id     = ""
    client_secret = ""
    secret_key    = ""
  }

  sensitive = true
}

/***********************
** NON-SECRET VARIABES
************************/
variable "deployment_environment" {
  type = string

  default = "production"
}

variable "name_suffix" {
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
  default = "12.9"
}

variable "database_name" {
  type    = string
  default = "galaxy"
}

variable "database_username" {
  type    = string
  default = "galaxy"
}

variable "database_final_snapshot_identifier" {
  type    = string
  default = "bye-underpass"
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

variable "tasking-manager_vpc_id" {
  type    = string
  default = "vpc-ea28198f"
}

variable "dump_path" {
  type    = string
  default = "/var/log/galaxy-api"
}

variable "container_image_uri" {
  type    = string
  default = "quay.io/hotosm/galaxy-api:container"
}

variable "alb_tls_policy" {
  type    = string
  default = "ELBSecurityPolicy-FS-1-2-Res-2020-10"
}

variable "api_host" {
  type    = string
  default = "galaxy-api.hotosm.org"
}

variable "api_port" {
  type    = string
  default = "443"
}

variable "api_url_scheme" {
  type    = string
  default = "https://"
}
