FROM python:3.9

LABEL maintainer="Humanitarian OpenStreetMap Team" Description="This image provides the Underpass API" Vendor="HOT" Version="dev"

WORKDIR /code

RUN apt-get update && apt-get -y install \ 
    postgresql \
    libpq-dev

COPY ./python/dbapi /code/api/dbapi
COPY ./python/restapi /code/api/restapi

RUN pip3 install -r /code/api/dbapi/requirements.txt
RUN pip3 install -r /code/api/restapi/requirements.txt

WORKDIR /code/api/restapi

ENTRYPOINT ["uvicorn", "main:app", "--host", "0.0.0.0", "--port", "8000"]

