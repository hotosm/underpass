terraform {
  required_providers {
    aws = {
      source  = "hashicorp/aws"
      version = "~> 3.0"
    }
    random = {
      source  = "hashicorp/random"
      version = "3.1.0"
    }
  }
}

provider "aws" {
  region = var.aws_region
  default_tags {
    tags = {
      Project       = "underpass"
      Maintainer    = "Dakota Benjamin & Yogesh Girikumar"
      Documentation = "https://docs.hotosm.org/underpass_infra"
    }
  }
}

provider "random" {
  # config options
}


