FROM node:alpine 

LABEL maintainer="Humanitarian OpenStreetMap Team" Description="This image provides the Underpass UI playground" Vendor="HOT" Version="dev"

WORKDIR /code

COPY ./js /code

RUN yarn install

ENTRYPOINT ["yarn", "cosmos"]

