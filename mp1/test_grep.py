#!/usr/bin/python
import threading
import os
from pexpect import pxssh
import time
import random
import datetime
import string

#############gen log##################################
def gen_log():
    return '{source} - - [{visit_time} -4000] "GET {path} HTTP/1.1" 200 {ret_code}'.format(
        visit_time=random_date(),
        path=gen_path(),
        ret_code=random.randint(1000, 23333),
        source=gen_source())


def random_char():
    return ''.join(
        random.choice(string.ascii_lowercase)
        for x in range(random.randint(3, 8)))


def gen_path():
    return '/' + '/'.join([random_char() for i in range(random.randint(0, 5))])


def gen_source():
    suffix = ['com', 'net', 'cn', 'edu', 'org']
    return '.'.join([random_char() for i in range(random.randint(2, 4))
                     ]) + '.' + random.choice(suffix)


def random_date():
    start = datetime.datetime(year=2000, month=5, day=24)
    end = datetime.datetime(year=2013, month=5, day=24)
    rand_date = start + \
        datetime.timedelta(seconds=random.randint(
            0, int((end - start).total_seconds())))
    return rand_date.strftime("%d/%b/%Y:%H:%M:%S")



#################test###############################
sum=0 #definition
def grep_all(ith,keyword,filename,dirname):
	global sum
	command_grep='pssh -i -H zijunc2@fa17-cs425-g59-' + str(ith).zfill(2) + r' grep "' +  keyword + '" /usr/local/mp1/' + filename + str(ith) + '.log > ./' +dirname+'/tgrep' + str(ith)
	#print command_grep
	os.system(command_grep)
	command_count='wc -l ./' +dirname+'/tgrep' + str(ith)
	count_grep_result=os.popen(command_count).readline() #the real count should be the reported num -1
	count_tmp=str(count_grep_result).split(" ")
	count=int(count_tmp[0])-1
	out_grep='echo "Grep '+str(count)+' lines from machine ' + str(ith) + '" >> ./' +dirname+'/count_grep'
	#print out_grep
	os.system(out_grep)
	sum+=count


def other_oprtabt_grep(keyword,filename,dirname):
	#keyword is the word to be grep
	#filename is the common part of the data filename on each vm
	# like "vm" - the whole filename would be vmi.log,i=1...10 
	global sum
		
	#make sure this files are clean
	f=open('./' +dirname+'/count_grep','w')
	f.close()
	f=open('./' +dirname+'/count_dgrep','w')
	f.close()
	
	sum = 0	#make sure sum is 0 before each  test
	for i in range(1,11):
		t=threading.Thread(target=grep_all,args=(i,keyword,filename,dirname))
                t.start()
		t.join()
	#os.system('echo "Total: ' + str(sum) + '" >> count_grep')	
	os.system('echo "' + str(sum) + '" >> ./' +dirname+'/count_grep')	
	
	#print grep & dgrep result
	os.system('./dgrep "'+ keyword + '" '+ filename + '.log > ./' +dirname+'/count_dgrep')
	print('------grep "'+keyword+'" result------')
	os.system('cat ./' +dirname+'/count_grep')
	print('\n------dgrep "'+keyword+'" result------')
	os.system('cat ./' +dirname+'/count_dgrep')


def test(keyword,filename,dirname):
	'''
	#keyword is the word to be grep
	#filename is the common part of the data filename on each vm
	# like "vm" - the whole filename would be vmi.log,i=1...10 
	global sum
		
	#make sure this files are clean
	f=open('./' +dirname+'/count_grep','w')
	f.close()
	f=open('./' +dirname+'/count_dgrep','w')
	f.close()
	
	sum = 0	#make sure sum is 0 before each  test
	for i in range(1,11):
		t=threading.Thread(target=grep_all,args=(i,keyword,filename,dirname))
                t.start()
	#os.system('echo "Total: ' + str(sum) + '" >> count_grep')	
	os.system('echo "' + str(sum) + '" >> ./' +dirname+'/count_grep')	
	
	#print grep & dgrep result
	os.system('./dgrep "'+ keyword + '" '+ filename + '.log > ./' +dirname+'/count_dgrep')
	print('------grep "'+keyword+'" result------')
	os.system('cat ./' +dirname+'/count_grep')
	print('\n------dgrep "'+keyword+'" result------')
	os.system('cat ./' +dirname+'/count_dgrep')
	'''
	other_oprtabt_grep(keyword,filename,dirname)
	#compare result
	fg=open('./' +dirname+'/count_grep','r')
	fdg=open('./' +dirname+'/count_dgrep','r')
	try:
		flag=True
		for i in range(10):
			line_g=fg.readline()
			line_dg=fdg.readline()
			if line_g and line_dg:
				count_g=str(line_g).split(" ")[1]
				count_dg=str(line_dg).split('Grep')[1].split('lines')[0].strip()
				#print count_g+" "+count_dg
				if cmp(count_g,count_dg) != 0:
					print 'Find difference between grep & dgrep result on machine',str(i)
					flag=False
					#return -1
					
			else:
				flag=False
				break
		
		line_g=fg.readline()
		line_dg=fdg.readline()
		if line_g and line_dg:
			count_g=str(line_g)
			count_dg=str(line_dg)
			#print count_g+" "+count_dg
		else:
			print 'Find difference between grep & dgrep sum result' 
			flag=False
		
		
	finally:
		fg.close()
		fdg.close()
		if(flag==True):
			print 'Succeed in test of grep "'+keyword+'"\n'
		else:	
			print 'Failed in test of grep "'+keyword+'"\n'


def data_gen(ith,filename):#gen data to 10 vms

	#make sure this files are clean
	f=open('testdata'+str(ith)+'.log','w')
	f.close()
	for i in range(500):
		os.system("echo \'"+gen_log()+"\' >> /usr/local/mp1/"+filename+str(ith)+'.log')	
	if ith>1:
		os.system('scp '+filename+str(ith)+'.log'+' zijunc2@fa17-cs425-g59-'+str(ith).zfill(2)+':/usr/local/mp1/')
		os.system('rm '+filename+str(ith)+'.log')
def data_gen_all():
	#'''	
	#gen data to 10 vms
	for i in range(1,11):
		t=threading.Thread(target=data_gen,args=(i,"testdata"))
		t.start()
		t.join()
	#'''


def main():	
	t=threading.Thread(target=data_gen_all)
	t.start()
	t.join()

	ISOTIMEFORMAT="-%Y-%m-%d-%X"
	dirname='testgrep'+time.strftime(ISOTIMEFORMAT,time.localtime())
	#print 'mkdir testgrep'+dirsuffix	
	os.system('mkdir '+dirname)
	
	
		
	test("HTTP","testdata",dirname)#frequent
	test(".org","testdata",dirname)#sometimes
	test("/Jun","testdata",dirname)#sometimes
	test("23/Apr","testdata",dirname)#sometimes
	test("198","testdata",dirname)#sometimes
	test("Nov/2007:01","testdata",dirname)#sometimes
	test("uffba.msfyqmhq.xzvqj.elc.net","testdata",dirname)#rare

main()


