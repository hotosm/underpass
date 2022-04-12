; Insights database access credentials
; TODO: None
[INSIGHTS]
host=${insights_pg_host}
user=${insights_pg_user}
password=${insights_pg_password}
database=${insights_pg_database}
port=${insights_pg_port}

; Underpass database access credentials
; TODO: None
[UNDERPASS]
host=${underpass_pg_host}
user=${underpass_pg_user}
password=${underpass_pg_password}
database=${underpass_pg_database}
port=${underpass_pg_port}

; Tasking Manager database access credentials
; TODO: Rename section title
[TM]
host=${tasking_manager_pg_host}
user=${tasking_manager_pg_user}
password=${tasking_manager_pg_password}
database=${tasking_manager_pg_database}
port=${tasking_manager_pg_port}

; OAuth2 Credentials for OSM.org login
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

