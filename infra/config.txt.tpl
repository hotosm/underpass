[DATABASE]
host=${pg_host}
user=${pg_user}
password=${pg_password}
dbname=${pg_database}
port=${pg_port}

[OAUTH]
client_id=${oauth2_client_id}
client_secret=${oauth2_client_secret}
redirect_uri=http://127.0.0.1:8000/callback
url=https://www.openstreetmap.org
scope=read_prefs
login_redirect_uri=http://127.0.0.1:8000/auth/callback
secret_key=${oauth2_secret_key}

[DUMP]
path=${dump_path}
underpass=underpass.sql
osmstats=osmstats.sql

[INSIGHTS]
host=${insights_pg_host}
user=${insights_pg_user}
password=${insights_pg_password}
database=${insights_pg_database}
port=${insights_pg_port}

[UNDERPASS]
host=${underpass_pg_host}
user=${underpass_pg_user}
password=${underpass_pg_password}
database=${underpass_pg_database}
port=${underpass_pg_port}
