For compilation problems with
ld not finding stdc++fs you should install newest stdc++
and then exec this.

sudo ln -s /usr/lib/gcc/x86_64-linux-gnu/8/libstdc++.so /usr/lib/libstdc++.so
sudo ln -s /usr/lib/gcc/x86_64-linux-gnu/8/libstdc++fs.a /usr/lib/libstdc++fs.a