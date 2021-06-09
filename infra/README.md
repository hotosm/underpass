# Underpass - Infrastructure Stack

This directory / repository contains code that spins up the infrastructure
necessary to run Underpass.

![Underpass architecture diagram](/images/underpass.png)

The stack consists of 

- A PostgreSQL RDS database with 1TB of disk space
- An S3 bucket to store backups of the database
- An instance that runs the C++ binaries that process data
- An instance that runs the Python API service
- CloudWatch logging and metrics collection
- CloudWatch alerts
- CloudFront CDN distribution that reads from the API service
- Route53 DNS entries (A or ALIAS) pointing to the CloudFront distribution

## A (very) brief, basic introduction to Terraform

We use terraform to spawn our infrastructure. We can use JSON or the Hashicorp
declarative DSL called Hashicorp Configuration Language (HCL) to declare the
state of the infrastructure that we can then use to deploy the infrastructure,
track changes to the infrastructure, apply the diff non-destructively if we 
make changes and even destroy the infrastructure.

The state of the current deployed infrastructure is stored in statefiles with
extension `.tfstate`. These are the sources of truths and must not be meddled
with. In order to collaboratively work with statefiles, we can use a shared
backend to store the statefile to which other collaborators would have access.

In our case, we use the terraform cloud that has all the bells and whistles
needed to maintain our infrastructure state files.

We use the following version of terraform to run our code

```
~/underpass $ terraform -v
Terraform v1.0.0
on linux_amd64
+ provider registry.terraform.io/hashicorp/aws v3.42.0
```

# Deploying the infrastructure

* Get session token

This is a temporary token that can be used to deploy resources on-to AWS. It
expires in one hour by default.

```
~/underpass » aws sts get-session-token --output json
{
    "Credentials": {
        "AccessKeyId": "ASIAZYDVV4ILEMQV3H57",
        "SecretAccessKey": "62Xm7KKhpljQJEC0yppSZz0fDW3SYc/QMT99J89l",
        "SessionToken": "FwoGZXIvYXdzE...sbQFWqz89I11mafgWQ5gBue6o=",
        "Expiration": "2021-06-02T21:37:19Z"
    }
}
```

* Assume role

Session tokens can't be used to invoke IAM API. We need more privileges for it.
We use STS assume-role to gain more privileged - but still temporary - access to
AWS API. 


```
aws sts assume-role --serial-number arn:aws:iam::670261699094:mfa/yogeshgirikumar --token-code 729586 --role-arn arn:aws:iam::670261699094:role/yogesh-terraform-assumeRole --role-session-name terraform1 --output json
{
    "Credentials": {
        "AccessKeyId": "ASIAZYDVV4ILI4C4EWHX",
        "SecretAccessKey": "S2UK+XuLq2ZZo1jlEhu+QU9tQVGdrvwTQ/fciu59",
        "SessionToken": "FwoGZXIvYXdzEND//////////wEaDPSm5TBmApFmXm2qHyKuASPdPvoP74pkzF+ebQaaPRC1y0xQfx2eOGNgVZkx0MHaOc23a0SE7tX6+FnmrNFf0LN1dm3qiPjxGV1cfGGsipURtfY4KRxpEBhtdamqCr35YZ1X5f0Ftr8xzHdFGNDw0YR9pywHaFCvtS355bFPcV2BMl5Cdt3NztHntx2okd0Jo9ljxmfj3Mz05nXOvNah061JlM2K35m6actdc6EaTjiaYbKMNSnxJcOG8mCCOSj8/P2FBjIt5EeuLM/3pZzVn2co6iqnZla8voQBTRm01DKhY85EusbjJA3HzwyX8oRJfQda",
        "Expiration": "2021-06-08T15:28:12Z"
    },
    "AssumedRoleUser": {
        "AssumedRoleId": "AROAZYDVV4ILBBV7CR2B5:terraform1",
        "Arn": "arn:aws:sts::670261699094:assumed-role/yogesh-terraform-assumeRole/terraform1"
    }
}
```

Set these as `AWS_ACCESS_KEY_ID`, `AWS_SECRET_ACCESS_KEY`, and `AWS_SESSION_TOKEN`
respectively in the Environment Variables section for Terraform cloud

* Initialize the modules

Run `terraform init` to download the modules and providers we use for our code

* Plan the infrastructure

Manually run `terraform plan` on your workstation or setup automatic
infrastructure planning on Terraform cloud triggered by pushes to the GitHub
repository.

* Apply the plan

If you are satisfied with the infrastructure plan and all looks good, then apply
the infrastructure by running `terraform apply`


# LICENSE

TO BE DECIDED

## Contribution guidelines

- Format and validate terraform code before submission

```
terraform fmt .
terraform validate .
# Run a plan to see if the changes look fine
terraform plan -compact-warnings
```

## Troubleshooting
