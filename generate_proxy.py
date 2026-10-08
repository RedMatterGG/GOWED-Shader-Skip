from pathlib import Path
import struct,json,hashlib
root=Path(__file__).parent
p=Path('C:/Windows/System32/dwmapi.dll');data=p.read_bytes();pe=struct.unpack_from('<I',data,60)[0]
n=struct.unpack_from('<H',data,pe+6)[0];opt=struct.unpack_from('<H',data,pe+20)[0];sections=[]
for i in range(n):sections.append(struct.unpack_from('<IIII',data,pe+24+opt+40*i+8))
def off(rva):
 for vs,va,rs,raw in sections:
  if va<=rva<va+max(vs,rs):return raw+rva-va
 raise ValueError(hex(rva))
def text(rva):
 o=off(rva);return data[o:data.index(b'\0',o)].decode('ascii')
export=struct.unpack_from('<I',data,pe+24+112)[0];e=off(export)
base,fc,nc,fr,nr,orr=struct.unpack_from('<IIIIII',data,e+16)
byordinal={}
for i in range(nc):
 r=struct.unpack_from('<I',data,off(nr)+4*i)[0];o=struct.unpack_from('<H',data,off(orr)+2*i)[0]+base;byordinal[o]=text(r)
exports=[]
for i in range(fc):
 r=struct.unpack_from('<I',data,off(fr)+4*i)[0]
 if r:exports.append({'ordinal':base+i,'name':byordinal.get(base+i)})
(root/'proxy_manifest.json').write_text(json.dumps({'source':str(p),'sha256':hashlib.sha256(data).hexdigest(),'exports':exports},indent=2))
(root/'proxy_exports.h').write_text('#pragma once\nconstexpr size_t exportCount='+str(len(exports))+';\nconstexpr const char* exportNames[]={'+','.join('"'+e['name']+'"' if e['name'] else 'nullptr' for e in exports)+'};\nconstexpr WORD exportOrdinals[]={'+','.join(str(e['ordinal']) for e in exports)+'};\n')
(root/'dwmapi.def').write_text('LIBRARY dwmapi\nEXPORTS\n'+''.join(f" {e['name'] or 'ordinal'+str(e['ordinal'])}=f{i} @{e['ordinal']}"+(' NONAME' if not e['name'] else '')+'\n' for i,e in enumerate(exports)))
asm=['option casemap:none','.code','extern mProcs:QWORD','extern EnsureForwarders:PROC']
for i in range(len(exports)):asm.extend([f'f{i} proc',f' mov r11d,{i}',' jmp ForwardDispatch',f'f{i} endp'])
asm.extend(['ForwardDispatch proc frame',' sub rsp,0A8h',' .allocstack 0A8h',' .endprolog',' mov [rsp+20h],rcx',' mov [rsp+28h],rdx',' mov [rsp+30h],r8',' mov [rsp+38h],r9',' mov [rsp+40h],r11',' movdqu [rsp+50h],xmm0',' movdqu [rsp+60h],xmm1',' movdqu [rsp+70h],xmm2',' movdqu [rsp+80h],xmm3',' call EnsureForwarders',' mov r11,[rsp+40h]',' lea rax,mProcs',' mov rax,[rax+r11*8]',' mov rcx,[rsp+20h]',' mov rdx,[rsp+28h]',' mov r8,[rsp+30h]',' mov r9,[rsp+38h]',' movdqu xmm0,[rsp+50h]',' movdqu xmm1,[rsp+60h]',' movdqu xmm2,[rsp+70h]',' movdqu xmm3,[rsp+80h]',' add rsp,0A8h',' jmp rax','ForwardDispatch endp','end'])
(root/'dwmapi.asm').write_text('\n'.join(asm)+'\n')
print('Generated',len(exports),'real dwmapi exports; source SHA256',hashlib.sha256(data).hexdigest())
