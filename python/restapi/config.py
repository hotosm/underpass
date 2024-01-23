import os

UNDERPASS_DB=os.getenv("UNDERPASS_API_DB") or "postgresql://localhost/underpass"
ORIGINS = os.getenv("UNDERPASS_API_ORIGINS").split(",") if os.getenv("UNDERPASS_API_ORIGINS") else [
    "http://localhost",
    "http://localhost:5000",
    "http://localhost:3000",
    "http://127.0.0.1",
    "http://127.0.0.1:5000"
]
AVAILABILITY=os.getenv("UNDERPASS_API_AVAILABILITY").split(",") if os.getenv("UNDERPASS_API_AVAILABILITY") else []