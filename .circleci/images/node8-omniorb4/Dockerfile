FROM circleci/node:8

# install OmniORB 4
RUN sudo apt-get update \
  && sudo apt-get install libomniorb4-dev

# delete all the apt list files since they're big and get stale quickly
RUN sudo rm -rf /var/lib/apt/lists/*
# this forces "apt-get update" in dependent images, which is also good
