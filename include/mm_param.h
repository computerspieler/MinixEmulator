/* The following names are synonyms for the variables in the input message. */
#define addr		m1_p1
#define exec_name	m1_p1
#define exec_len	m1_i1
#define func		m6_f1
#define grpid		(gid_t) m1_i1
#define namelen		m1_i1
#define pid			m1_i1
#define seconds		m1_i1
#define sig			m6_i1
#define stack_bytes	m1_i2
#define stack_ptr	m1_p2
#define status		m1_i1
#define usr_id		(uid_t) m1_i1
#define request		m2_i2
#define taddr		m2_l1
#define data		m2_l2
#define sig_nr		m1_i2
#define sig_nsa		m1_p1
#define sig_osa		m1_p2
#define sig_ret		m1_p3
#define sig_set		m2_l1
#define sig_how		m2_i1
#define sig_flags	m2_i2
#define sig_context	m2_p1
#ifdef _SIGMESSAGE
#define sig_msg		m1_i1
#endif
#define reboot_flag	m1_i1
#define reboot_code	m1_p1
#define reboot_size	m1_i2
#define svrctl_req	m2_i1
#define svrctl_argp	m2_p1

/* The following names are synonyms for the variables in a reply message. */
#define reply_res	m_type
#define reply_res2	m2_i1
#define reply_ptr	m2_p1
#define reply_mask	m2_l1 	

