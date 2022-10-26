terraform {
  required_version = ">= 1.3.2"

  backend "remote" {
    organization = "hotosm"

    workspaces {
      name = "Galaxy"
    }
  }

  required_providers {
    aws = {
      source  = "hashicorp/aws"
      version = "~> 4.36.1"
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
    tags = {
      Project         = "galaxy"
      Maintainer      = "DK_Benjamin and Yogesh_Girikumar"
      Documentation   = "https://docs.hotosm.org/galaxy_infra"
      Management_Mode = "Terraform workspace Galaxy" // or Manual
    }
  }
}

provider "random" {
  # config options
}


