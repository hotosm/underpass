terraform {
  required_version = ">= 1.4.0"

  # backend "remote" {
  #   organization = "hotosm"

  #   workspaces {
  #     name = "Galaxy"
  #   }
  # }

  required_providers {
    aws = {
      source  = "hashicorp/aws"
      version = "~> 5.1.0"
    }

    random = {
      source  = "hashicorp/random"
      version = "3.4.3"
    }
  }
}

provider "aws" {
  region = var.aws_region
  default_tags {
    # tags = var.resource_tags
  }
}

provider "random" {
  # config options
}


