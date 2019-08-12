Steps to creating a docker image.

    sudo docker run -it -d --name quantel-gateway -p 3000:3000 circleci/node:8 bash
    docker attach quantel-gateway

Then, in the container ...

    cd /home/circleci
    sudo apt-get update
    sudo apt-get install libomniorb4-dev
    git clone https://github.com/nrkno/tv-automation-quantel-gateway.git
    cd tv-automation-quantel-gateway
    git checkout develop
    yarn install
    yarn build

Can then test with the node REPL or by running the server with `yarn server`.

Exit docker shell with Ctrl-P Ctrl-Q and commit the image ...

    sudo docker images
    sudo docker commit <image-id> sofietv/tv-automation-quantel-gateway:v42

Stop and optionally remove the build container:

    sudo docker stop quantel-gateway
    sudo docker rm quantel-gateway

Run up the server:

    sudo docker run -it -w /home/circleci/tv-automation-quantel-gateway --name quantel-gateway -p 3000:3000 sofietv/tv-automation-quantel-gateway:v42 sh -c "yarn server"
