#!/bin/bash
docker run -t --name testmg -e VAULT_ADDR=$VAULT_ADDR -e VAULT_TOKEN=$VAULT_TOKEN -e LEIN_ROOT=t \
       -e AWS_ACCESS_KEY_ID=$AWS_ACCESS_KEY_ID -e AWS_SECRET_ACCESS_KEY=$AWS_SECRET_ACCESS_KEY \
       -e LEIN_USERNAME=$AWS_ACCESS_KEY_ID -e LEIN_PASSWORD=$AWS_SECRET_ACCESS_KEY -e LEIN_PASSPHRASE=$AWS_SECRET_ACCESS_KEY \
       --volume=`pwd`:/root/build hdf5-buildimg /bin/bash -c "cd /root/build && lein test && lein deploy"
