# KB-PM
a CLI tool (like supervisor and pm2) keep a group of programs running continuously.

## Screenshot
![](./data/pic.png)<br>

## Installation
``` shell
git clone https://github.com/nladuo/KB-PM.git
cd KB-PM && make && sudo make install
```
## Usage
``` shell
kbpm service start              # start the KB_PM service
chmod +x ./bin/test.sh          # make test.sh executeable
kbpm start ./bin/test.sh        # execute test.sh
tail -f test.log                # check out the stdout of test.sh
```

## License
MIT