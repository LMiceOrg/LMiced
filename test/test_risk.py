#coding: utf-8
import lmspi
import time
import struct
import ctypes
import random
fac = lmspi.LMFactory()
spi = fac.create_spi()
tid = lmspi.LMEvent()
tid2 = lmspi.LMEvent()
bound_id = lmspi.LMEvent()
bound_msg = lmspi.LMMessage()
stock_id = lmspi.LMEvent()
stock_msg = lmspi.LMMessage()
warning_id = lmspi.LMEvent()
warning_msg = lmspi.LMMessage()
rate_id = lmspi.LMEvent()
rate_msg = lmspi.LMMessage()

begin =ctypes.c_int64()
end =ctypes.c_int64()
pb = ctypes.pointer(begin)
pe = ctypes.pointer(end)

def func_callback(id):
	bound=bound_msg.set(struct.pack('d', 120000.0+random.random()*500))
	stock_msg.set(struct.pack('d', 60000.0+random.random()*30000))
	ctypes.windll.kernel32.GetSystemTimeAsFileTime(pb)
	print "开始风险模型计算:",time.ctime()
	bound=bound_msg.get(0)
	bv = struct.unpack('d', bound[0:8])[0]
	print "\t1. 获取保证金信息", bv
	
	stock=stock_msg.get(0)
	sv = struct.unpack('d', stock[0:8])[0]
	print "\t2. 获取证券信息", sv
	rate = bv / (sv+0.1) * 100.0
	print "\t3. 计算充足率信息", rate
	rate_msg.set( struct.pack('d', rate))
	print "\t4. 发布保证金充足率信息"
	if rate < 0.1:
		print "\t5. 发布持仓警告信息"
		warning_msg.set( struct.pack('d', rate) )
	ctypes.windll.kernel32.GetSystemTimeAsFileTime(pe)
	tm = (end.value - begin.value)*1.0/1e4
	print "结束风险模型计算  用时 %f 毫秒\n" % tm

spi.init()
spi.join_session( ctypes.windll.kernel32.GetCurrentThreadId() )
spi.register_timer_event(10000000,0xffff,0,tid)
spi.register_callback(tid.getid(), func_callback)
spi.register_publish("performance bond", "account stock", 4096, bound_id)
spi.register_publish("stock", "account stock", 4096, stock_id)
spi.register_subscribe("bond rate", 		"risk model", rate_id)
spi.register_subscribe("account warning", 	"risk model", warning_id)

spi.get_mesg(bound_id, bound_msg)
spi.get_mesg(stock_id, stock_msg)

spi.get_mesg(rate_id, rate_msg)
spi.get_mesg(warning_id, warning_msg)

spi.commit()
spi.join()
