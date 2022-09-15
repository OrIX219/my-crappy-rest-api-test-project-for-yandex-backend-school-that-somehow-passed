Dependencies
------------
### Already in */include* folder ###
* [Asio](https://think-async.com/Asio/AsioStandalone.html) (boost::asio standalone version)
* [JSON for Modern C++](https://github.com/nlohmann/json)
* [date](https://github.com/HowardHinnant/date)
### Need installation ##
* [libpqxx](https://github.com/jtv/libpqxx)
In fact, installing some libpqxx package with your package manager should be enough.

Build
-----
You just:
```
make
```
And then the binary will magically appear in */bin* folder.
Or it will crash with some link errors or something idk. 
(Most likely because of libpqxx(If it happens it is certainly because of it))

Runinng
-------
### With Docker ###
If you have **docker** with **compose** plugin, after building you can also:
```
sudo docker compose up
```
But be sure that nothing else is running on **80**(api) and **5432**(db) ports.
It will build docker image with main binary and then start it and another container with postgresql.
After that you will be able to access api on *0.0.0.0* or *127.0.0.1* or *localhost*.

### Without Docker ###
If you don' have **docker** then you need:
1. Install **postgresql**
2. Configure it to listen on *localhost*
3. Make some superuser with some password
4. Make new db
5. Run binary:
```
sudo bin/Main *127.0.0.1* *<<your db name>>* *<<your user>>* *<<your password>>*
```
What these args mean is pretty obvious except for first one:
it is ip addr where db is running.
If you want to pass *localhost*, I think for it to work you need to change *hostaddr* string in DbManager constructor in ***DbManager.cpp*** to just *host* and rebuild binary. I didn't try, idc.
