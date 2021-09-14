from japronto import Application
from japronto.request import HttpRequest

def RP(request: HttpRequest):
    r = request
    body = r.body.decode ()
    print ("POC -> recv msg: ", body)
    return r.Response(text=body)

app = Application()
app.router.add_route('/poc', RP)
app.run(debug=True, port=9999)