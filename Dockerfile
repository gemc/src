# SCI-G Continuous Integration

# Container image that runs your code
FROM jeffersonlab/gemc:3.0

# Copies your code file from your action repository to the filesystem path `/` of the container
COPY ci.sh /ci.sh

# Code file to execute when the docker container starts up (`entrypoint.sh`)
ENTRYPOINT ["/ci.sh"]
