import re,json,base64,numpy as np
from PIL import Image,ImageDraw
from scipy import ndimage
SC=5712/1600
src=open('/home/claude/led-logo-firmware/src/main.cpp').read()
a=src.index('const uint16_t segments');b=src.index('};',src.index('// Char "O"'))
chars=[]
for blk in re.split(r'\{ // ',src[a:b])[1:]:
    cols=[[int(n) for n in re.findall(r'\d+',c)] for c in re.findall(r'\{([^{}]*)\}',blk)]
    chars.append([c for c in cols if c])
full=Image.open('photos/full.png').convert('RGB')
# work at half res
H2=full.resize((2856,2142))
f=2856/1600
hsv=np.asarray(H2.convert('HSV')).astype(int)
mask=hsv[:,:,2]>110; mask[:int(400*f)]=False; mask[int(760*f):]=False
mask=ndimage.binary_opening(mask,iterations=2)
dist=ndimage.distance_transform_edt(mask)
# init from previous
p=json.load(open('pos.json'))
X0,Y0=146,464
P={int(n):np.array([(v[0]*1.3+X0)*f,(v[1]*1.3+Y0)*f]) for n,v in p.items()}
ids=sorted(P);idx={n:i for i,n in enumerate(ids)}
X=np.array([P[n] for n in ids],float)
charof={};colof={}
for ci,ch in enumerate(chars):
    for k,c in enumerate(ch):
        for n in c: charof[n]=ci;colof[n]=(ci,k)
cons=[(idx[n],idx[n+1]) for n in ids if n+1 in idx and charof[n]==charof[n+1]]
d0=np.array([np.linalg.norm(X[i]-X[j]) for i,j in cons])
pitch=np.median(d0);print('pitch',pitch)
cons=[c for c,dd in zip(cons,d0) if dd<2.5*pitch]
print('springs',len(cons))
margin=0.3*pitch
inner=dist>=margin
_,ind=ndimage.distance_transform_edt(~inner,return_indices=True)
cols={}
for n in ids: cols.setdefault(colof[n],[]).append(idx[n])
colorder={key:[idx[n] for n in chars[key[0]][key[1]]] for key in cols}
for it in range(400):
    F=np.zeros_like(X)
    for i,j in cons:
        v=X[j]-X[i];L=np.linalg.norm(v)+1e-6;e=(L-pitch)/L*v*0.25
        F[i]+=e;F[j]-=e
    D=X[:,None,:]-X[None,:,:];R=np.linalg.norm(D,axis=2)+np.eye(len(X))*1e9
    close=R<0.85*pitch
    rep=(D/R[:,:,None])*((0.85*pitch-R)*close)[:,:,None]*0.3
    F+=rep.sum(1)
    for key,lst in cols.items():
        mx=X[lst,0].mean();F[lst,0]+=(mx-X[lst,0])*0.08
    X+=F
    # keep inside
    xi=np.clip(X[:,0].round().astype(int),0,mask.shape[1]-1);yi=np.clip(X[:,1].round().astype(int),0,mask.shape[0]-1)
    out=~inner[yi,xi]
    X[out,1]=ind[0][yi[out],xi[out]];X[out,0]=ind[1][yi[out],xi[out]]
    # keep column order top->bottom
    for key,lst in colorder.items():
        ys=sorted(X[lst,1]);X[lst,1]=ys
d1=np.array([np.linalg.norm(X[i]-X[j]) for i,j in cons]);print('spring dev before %.2f after %.2f'%(np.std(d0[d0<2.5*pitch])/pitch,np.std(d1)/pitch))
img=H2.copy();dr=ImageDraw.Draw(img)
for (x,y),n in zip(X,ids):
    dr.ellipse([x-9,y-9,x+9,y+9],outline='white',width=3)
for i,j in cons: dr.line([tuple(X[i]),tuple(X[j])],fill=(255,255,255),width=1)
bx0,by0=int(120*f),int(430*f)
img.crop((bx0,by0,int(1480*f),int(760*f))).resize((1600,int(330*f*1600/(1360*f)))).save('overlay2.png')
# normalize to 0..1000
xs=X[:,0];ys=X[:,1];x0,x1=xs.min(),xs.max();y0=ys.min()
s=1000/(x1-x0)
out={str(n):[round((x-x0)*s),round((y-y0)*s)] for (x,y),n in zip(X,ids)}
json.dump(out,open('pos2.json','w'));print('H',max(v[1] for v in out.values()))
# silueta loga pro nahled: bitmapa v souradnicich LED, 4 jednotky / px
U=4;PADU=24
W=int((1000+2*PADU)/U);Hh=int((max(v[1] for v in out.values())+2*PADU)/U)
bm=np.zeros((Hh,W),bool)
for yy in range(Hh):
    for xx in range(W):
        ux=xx*U-PADU+U/2;uy=yy*U-PADU+U/2
        px=int(ux/s+x0);py=int(uy/s+y0)
        if 0<=py<mask.shape[0] and 0<=px<mask.shape[1]: bm[yy,xx]=mask[py,px]
Image.fromarray((bm*255).astype('uint8')).save('sil.png')
bits=np.packbits(bm.flatten())
json.dump({'w':W,'h':Hh,'u':U,'pad':PADU,'b64':base64.b64encode(bits.tobytes()).decode()},open('sil.json','w'))
print('sil',W,Hh,len(base64.b64encode(bits.tobytes())))
