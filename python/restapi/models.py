from pydantic import BaseModel
from typing import Union

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
    area: Union[str, None] = None
    tags: str = None
    hashtag: str = None
    dateFrom: str = None
    dateTo: str = None
    status: str = None
    page: int = None

class StatsRequest(BaseModel):
    area: Union[str, None] = None
    tags: str = None
    hashtag: str = None
    dateFrom: str = None
    dateTo: str = None
    status: str = None

