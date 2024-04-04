variable "project_name" {
  type = string

  default = "underpass"
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

variable "galaxy_db_credentials" {
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
    database       = "db.r6g.xlarge"
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
  default = "underpass"
}

variable "database_username" {
  type    = string
  default = "underpass"
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
    db_min               = 1000
    db_max               = 5000
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

  validation {
    condition     = length(var.tasking-manager_vpc_id) > 11 && substr(var.tasking-manager_vpc_id, 0, 4) == "vpc-"
    error_message = "The VPC ID invalid"
  }
}

variable "container_image_uri" {
  type    = string
  default = "quay.io/hotosm/underpass-api"
}

variable "container_image_tag" {
  type    = string
  default = "container"

  description = "Git commit / release tag of the API that needs to be deployed"
}

variable "alb_tls_policy" {
  type    = string
  default = "ELBSecurityPolicy-FS-1-2-Res-2020-10"

  validation {
    condition     = length(var.alb_tls_policy) > 18 && substr(var.alb_tls_policy, 0, 18) == "ELBSecurityPolicy-"
    error_message = "The SecurityPolicy identifier is invalid"
  }
}

variable "api_host" {
  type    = string
  default = "underpass-api.hotosm.org"
}

variable "api_port" {
  type    = string
  default = "443"
}

variable "api_export_max_area" {
  type    = string
  default = "3000000"
}

variable "api_log_level" {
  type    = string
  default = "info"

  validation {
    condition     = contains(["info", "warning", "error", "debug"], var.api_log_level)
    error_message = "Invalid API Log Level"
  }
}

variable "api_url_scheme" {
  type    = string
  default = "https://"
}

variable "sentry" {
  type = map(string)
  default = {
    dsn             = ""
    app_release_tag = "underpass-api@v1.0.0"
  }

  description = "Data-source Name and app release tag to be displayed in Sentry.io dashboard"
}
