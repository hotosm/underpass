// output "underpass_instance_state" {
//   value = aws_instance.api[0].instance_state
// }

output "db-secret-arn" {
  value = aws_secretsmanager_secret.underpass_database_credentials.arn
}

output "db-secret-version-arn" {
  value = aws_secretsmanager_secret_version.underpass_database_credentials.arn
}

output "db-security-group" {
  value = aws_security_group.database.id
}
