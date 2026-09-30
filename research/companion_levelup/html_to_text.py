import sys,re,html
from html.parser import HTMLParser
class P(HTMLParser):
    def __init__(s):
        super().__init__(); s.o=[]; s.skip=0
    def handle_starttag(s,t,a):
        if t in('script','style'): s.skip+=1
        if t in('tr','p','br','h1','h2','h3','h4','li','div'): s.o.append('\n')
        if t in('td','th'): s.o.append(' | ')
    def handle_endtag(s,t):
        if t in('script','style'): s.skip-=1
    def handle_data(s,d):
        if not s.skip: s.o.append(d)
p=P(); p.feed(open(sys.argv[1],errors='ignore').read())
t=html.unescape(''.join(p.o)); t=re.sub(r'[ \t]+',' ',t); t=re.sub(r'\n\s*\n+','\n',t)
open(sys.argv[2],'w').write(t)
