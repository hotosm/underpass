import os

# ENABLE_UNDERPASS_CORE=True
UNDERPASS_DB=os.getenv("UNDERPASS_API_DB") or "postgresql://localhost/underpass"