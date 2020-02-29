import pexpect
import time
import threading
child_server = pexpect.spawn('./server')
n=15
l=[]
threads=[]
def spam(i):
    while(True):
      l[i].sendline("hello")

for i in range(n):
    l.append(pexpect.spawn('./client 8888 127.0.0.1'))

for i in range(1,len(l)-2):
    x=threading.Thread(target=spam,args=(i,),daemon=True)
    threads.append(x)
    x.start()

l[n-10].expect('connected')
start = time.time()
l[n-10].sendline('hello')
l[n-1].expect('hello')
end = time.time()
print(end - start)
