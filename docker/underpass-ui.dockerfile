FROM node:alpine 

LABEL maintainer="Humanitarian OpenStreetMap Team" Description="This image provides the Underpass UI playground" Vendor="HOT" Version="dev"

RUN apk --no-cache add git

WORKDIR /code
RUN git clone https://github.com/hotosm/underpass-ui.git .
RUN yarn install

ENTRYPOINT ["yarn", "cosmos"]
