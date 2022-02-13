from pyo import *

# make sure the length of jackname >= 31
s = Server(audio="jack", jackname="jackname_poc_testing_09-13-2021.").boot()

dens = Sig(0.5)
dens.ctrl(title="Density of grains per second")


pos = Randi(min=0.00, max=1.00, freq=0.1)

pit = Sig(0)
pit.ctrl(title="Transposition per grain")

dur = Sig(0.5)
dur.ctrl(title="Grain duration")

send = OscSend(
    input=[dens, pos, pit, dur],
    port=9000,
    address=["/density", "/position", "/pitch_rand", "/duration"],
    host="127.0.0.1",
)

s.gui(locals())