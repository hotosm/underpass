from pydantic import BaseModel

class DataQualityRequest(BaseModel):
    fromDate: str = None
    toDate: str = None
    hashtags: list = None
    area: str = None
    responseType: str = "json"

class OsmchangeValidateRequest(BaseModel):
    osmchange: str
    check: str
    
class RawRequest(BaseModel):
    area: str = None
    tags: str = None
    page: int = None
