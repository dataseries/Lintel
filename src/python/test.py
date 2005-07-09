import Lintel

foo = Lintel.new_Stats()
foo.add(14)
foo.add(12)
foo.add(10)
foo.add(13)

print "min=", foo.min()
print "max=", foo.max()
print "mean=", foo.mean()
print "conf95=", foo.conf95()

foo = Lintel.new_StatsQuantile()
foo.add(14)
foo.add(12)
foo.add(10)
foo.add(13)

print "min=", foo.min()
print "max=", foo.max()
print "mean=", foo.mean()
print "conf95=", foo.conf95()
print "quantile50=", foo.getQuantile(0.1)

print "clock=", Lintel.clock_rate()
print "now=", Lintel.clock_now()
print "now=", Lintel.clock_now()

