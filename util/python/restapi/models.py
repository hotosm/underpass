from pydantic import BaseModel

class DataQualityRequest(BaseModel):
    fromDate: str = None
    toDate: str = None
    hashtags: list = None
    area: str = None

class OsmchangeValidateRequest(BaseModel):
    osmchange: str
    check: str
    