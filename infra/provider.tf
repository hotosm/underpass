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
      Maintainer    = "Yogesh Girikumar"
      Documentation = "underpass-doc-stub"
    }
  }
}

provider "random" {
  # config options
}


