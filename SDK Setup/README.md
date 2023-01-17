https://circuitdigest.com/microcontroller-projects/how-to-program-raspberry-pi-pico-using-c

```
docker ps -aq | xargs docker rm; docker images --filter dangling=true -aq | xargs docker rmi; docker rmi pico-sdk; docker build -t pico-sdk --network host .
```