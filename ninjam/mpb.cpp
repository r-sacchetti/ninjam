#include <windows.h>

#include "mpb.h"



// MESSAGE_SERVER_AUTH_CHALLENGE 
int mpb_server_auth_challenge::parse(Net_Message *msg) // return 0 on success
{
  if (msg->get_type() != MESSAGE_SERVER_AUTH_CHALLENGE) return -1;
  int capsl=msg->get_size() - sizeof(challenge);
  if (capsl < 0) return 1;
  unsigned char *p=(unsigned char *)msg->get_data();
  if (!p) return 2;

  memcpy(challenge,p,sizeof(challenge));
  p+=sizeof(challenge);
  server_caps=0;
  int shift=0;


  if (capsl > 4) capsl=4;

  while (capsl--)
  {
    server_caps |= ((int)*p++)<<shift;
    shift+=8;
  }

  return 0;
}

Net_Message *mpb_server_auth_challenge::build()
{
  Net_Message *nm=new Net_Message;
  nm->set_type(MESSAGE_SERVER_AUTH_CHALLENGE);
  
  int scsize=0;
  int sc=server_caps;
  while (sc)
  {
    scsize++;
    sc>>=8;
  }

  nm->set_size(sizeof(challenge) + scsize);

  unsigned char *p=(unsigned char *)nm->get_data();

  if (!p) 
  {
    delete nm;
    return 0;
  }

  memcpy(p,challenge,sizeof(challenge));
  p+=sizeof(challenge);

  sc=server_caps;
  while (scsize--)
  {
    *p++=sc&0xff;
    sc>>=8;
  }

  return nm;
}



// MESSAGE_SERVER_AUTH_REPLY
int mpb_server_auth_reply::parse(Net_Message *msg) // return 0 on success
{
  if (msg->get_type() != MESSAGE_SERVER_AUTH_REPLY) return -1;
  if (msg->get_size() < 1) return 1;
  unsigned char *p=(unsigned char *)msg->get_data();
  if (!p) return 2;

  flag=*p;

  return 0;
}

Net_Message *mpb_server_auth_reply::build()
{
  Net_Message *nm=new Net_Message;
  nm->set_type(MESSAGE_SERVER_AUTH_REPLY);
  
  nm->set_size(1);

  unsigned char *p=(unsigned char *)nm->get_data();

  if (!p)
  {
    delete nm;
    return 0;
  }

  *p=flag;

  return nm;
}


// MESSAGE_SERVER_CONFIG_CHANGE_NOTIFY
int mpb_server_config_change_notify::parse(Net_Message *msg) // return 0 on success
{
  if (msg->get_type() != MESSAGE_SERVER_CONFIG_CHANGE_NOTIFY) return -1;
  if (msg->get_size() < 4) return 1;
  unsigned char *p=(unsigned char *)msg->get_data();
  if (!p) return 2;

  beats_minute = *p++;
  beats_minute |= ((int)*p++)<<8;
  beats_interval = *p++;
  beats_interval |= ((int)*p++)<<8;

  return 0;
}

Net_Message *mpb_server_config_change_notify::build()
{
  Net_Message *nm=new Net_Message;
  nm->set_type(MESSAGE_SERVER_CONFIG_CHANGE_NOTIFY);
  
  nm->set_size(4);

  unsigned char *p=(unsigned char *)nm->get_data();

  if (!p)
  {
    delete nm;
    return 0;
  }

  *p++=beats_minute&0xff;
  *p++=(beats_minute>>8)&0xff;
  *p++=beats_interval&0xff;
  *p++=(beats_interval>>8)&0xff;

  return nm;
}


// MESSAGE_SERVER_USERINFO_CHANGE_NOTIFY
int mpb_server_userinfo_change_notify::parse(Net_Message *msg) // return 0 on success
{
  if (msg->get_type() != MESSAGE_SERVER_USERINFO_CHANGE_NOTIFY) return -1;
  if (msg->get_size() < 1) return 1;

  m_intmsg = msg;
  return 0;
}

Net_Message *mpb_server_userinfo_change_notify::build()
{
  if (m_intmsg) 
  {
    Net_Message *n=m_intmsg;
    m_intmsg=0;
    return n;
  }

  Net_Message *nm=new Net_Message;
  nm->set_type(MESSAGE_SERVER_USERINFO_CHANGE_NOTIFY); 
  nm->set_size(0);

  return nm;
}


void mpb_server_userinfo_change_notify::build_add_rec(int isRemove, int channelid, 
                                                      int volume, int pan, int flags, char *username, char *chname)
{
  int size=1+ // is remove
           1+ // channel index
           4+ // volume
           1+ // pan
           1+ // flags
           strlen(username?username:"")+1+strlen(chname?chname:"")+1;

  if (!m_intmsg) m_intmsg = new Net_Message;
  int oldsize=m_intmsg->get_size();
  m_intmsg->set_size(size+oldsize);
  unsigned char *p=(unsigned char *)m_intmsg->get_data();
  if (p)
  {
    p+=oldsize;
    *p++=!!isRemove;
    
    if (channelid < 0) channelid=0;
    else if (channelid>255)channelid=255;
    *p++=channelid;

    *p++=volume&0xff;
    *p++=(volume>>8)&0xff;
    *p++=(volume>>16)&0xff;
    *p++=(volume>>24)&0xff;

    if (pan<-128) pan=-128;
    else if (pan>127)pan=127;
    *p++=(unsigned char)pan;

    *p++=(unsigned char)flags;

    strcpy((char*)p,username);
    p+=strlen(username)+1;
    strcpy((char*)p,chname);
    p+=strlen(chname)+1;
  }
}


// returns offset of next item on success, or <= 0 if out of items
int mpb_server_userinfo_change_notify::parse_get_rec(int offs, int *isRemove, int *channelid, int *volume, 
                                                     int *pan, int *flags, char **username, char **chname)
{
  int hdrsize=1+ // is remove
           1+ // channel index
           4+ // volume
           1+ // pan
           1; // flags

  if (!m_intmsg) return 0;
  unsigned char *p=(unsigned char *)m_intmsg->get_data();
  int len=m_intmsg->get_size()-offs;
  if (!p || len < hdrsize+2) return 0;
  p+=offs;

  unsigned char *hdrbuf=p;
  char *unp;
  char *cnp;

  if (len < hdrsize+2) return 0;
  hdrbuf=p;
  len -= hdrsize;
  unp=(char *)hdrbuf+hdrsize;
  cnp=unp;
  while (*cnp)
  {
    cnp++;
    if (!len--) return 0;
  }
  cnp++;
  if (!len--) return 0;

  p=(unsigned char *)cnp;
  while (*p)
  {
    p++;
    if (!len--) return 0;
  }
  p++;
  if (!len--) return 0;

  *isRemove=(int)*hdrbuf++;
  *channelid=(int)*hdrbuf++;
  *volume=(int)*hdrbuf++;
  *volume |= ((int)*hdrbuf++)<<8;
  *volume |= ((int)*hdrbuf++)<<16;
  *volume |= ((int)*hdrbuf++)<<24;
  *pan = (int) *hdrbuf++;
  *flags = (int) *hdrbuf++;

  *username = unp;
  *chname = cnp;


  return p - (unsigned char *)m_intmsg->get_data();
}


// MESSAGE_SERVER_DOWNLOAD_INTERVAL_BEGIN
int mpb_server_download_interval_begin::parse(Net_Message *msg) // return 0 on success
{
  if (msg->get_type() != MESSAGE_SERVER_DOWNLOAD_INTERVAL_BEGIN) return -1;
  if (msg->get_size() < 28+1) return 1;
  unsigned char *p=(unsigned char *)msg->get_data();
  if (!p) return 2;

  memcpy(guid,p,sizeof(guid));
  p+=sizeof(guid);
  estsize = (int)*p++;
  estsize |= ((int)*p++)<<8;
  estsize |= ((int)*p++)<<16;
  estsize |= ((int)*p++)<<24;
  fourcc = (int)*p++;
  fourcc |= ((int)*p++)<<8;
  fourcc |= ((int)*p++)<<16;
  fourcc |= ((int)*p++)<<24;
  transfer_id = (int)*p++;
  transfer_id |= ((int)*p++)<<8;
  chidx = (int)*p++;
  chidx |= ((int)*p++)<<8;
  int len=msg->get_size()-28;

  username=(char *)p;


  // validate null termination for now
  while (len)
  {
    if (!*p) break;
    p++;
    len--;
  }
  if (!len) return -1;

  return 0;
}


Net_Message *mpb_server_download_interval_begin::build()
{
  Net_Message *nm=new Net_Message;
  nm->set_type(MESSAGE_SERVER_DOWNLOAD_INTERVAL_BEGIN);
  
  nm->set_size(28+strlen(username?username:"")+1);

  unsigned char *p=(unsigned char *)nm->get_data();

  if (!p)
  {
    delete nm;
    return 0;
  }

  memcpy(p,guid,sizeof(guid));
  p+=sizeof(guid);
  *p++=(unsigned char)((estsize)&0xff);
  *p++|=(unsigned char)((estsize>>8)&0xff);
  *p++|=(unsigned char)((estsize>>16)&0xff);
  *p++|=(unsigned char)((estsize>>24)&0xff);
  *p++=(unsigned char)((fourcc)&0xff);
  *p++|=(unsigned char)((fourcc>>8)&0xff);
  *p++|=(unsigned char)((fourcc>>16)&0xff);
  *p++|=(unsigned char)((fourcc>>24)&0xff);
  *p++=(unsigned char)((transfer_id)&0xff);
  *p++|=(unsigned char)((transfer_id>>8)&0xff);
  *p++=(unsigned char)((chidx)&0xff);
  *p++|=(unsigned char)((chidx>>8)&0xff);

  strcpy((char *)p,username?username:"");


  return nm;
}