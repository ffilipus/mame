// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_bjt.h
 *
 */

#ifndef NLD_BJT_H_
#define NLD_BJT_H_

#include "nl_base.h"
#include "nld_twoterm.h"

// -----------------------------------------------------------------------------
// Macros
// -----------------------------------------------------------------------------

#define QBJT_SW(_name, _model)                                                 \
		NET_REGISTER_DEV(QBJT_SW, _name)                                       \
		NETDEV_PARAMI(_name,  MODEL,   _model)

#define QBJT_EB(_name, _model)                                                 \
		NET_REGISTER_DEV(QBJT_EB, _name)                                       \
		NETDEV_PARAMI(_name,  MODEL,   _model)


NETLIB_NAMESPACE_DEVICES_START()

// -----------------------------------------------------------------------------
// nld_Q - Base classes
// -----------------------------------------------------------------------------

// Have a common start for transistors

class NETLIB_NAME(Q) : public device_t
{
public:
	enum q_type {
		BJT_NPN,
		BJT_PNP
	};

	NETLIB_NAME(Q)(const family_t afamily, netlist_t &anetlist, const pstring &name);
	virtual ~NETLIB_NAME(Q)();

	inline q_type qtype() const { return m_qtype; }
	inline bool is_qtype(q_type atype) const { return m_qtype == atype; }
	inline void set_qtype(q_type atype) { m_qtype = atype; }
protected:
	virtual void start() override;
	virtual void reset() override;
	ATTR_HOT void update() override;

	param_model_t m_model;
private:
	q_type m_qtype;
};

class NETLIB_NAME(QBJT) : public NETLIB_NAME(Q)
{
public:

	NETLIB_NAME(QBJT)(const family_t afamily, netlist_t &anetlist, const pstring &name)
	: NETLIB_NAME(Q)(afamily, anetlist, name) { }

	virtual ~NETLIB_NAME(QBJT)() { }

protected:

private:
};




// -----------------------------------------------------------------------------
// nld_QBJT_switch
// -----------------------------------------------------------------------------


/*
 *         + -              C
 *   B ----VVV----+         |
 *                |         |
 *                Rb        Rc
 *                Rb        Rc
 *                Rb        Rc
 *                |         |
 *                +----+----+
 *                     |
 *                     E
 */

class NETLIB_NAME(QBJT_switch) : public NETLIB_NAME(QBJT)
{
public:
	ATTR_COLD NETLIB_NAME(QBJT_switch)(netlist_t &anetlist, const pstring &name)
	: NETLIB_NAME(QBJT)(BJT_SWITCH, anetlist, name),
		m_RB(object_t::ANALOG, anetlist, "m_RB"),
		m_RC(object_t::ANALOG, anetlist, "m_RC"),
		m_BC_dummy(object_t::ANALOG, anetlist, "m_BC"),
		m_gB(NETLIST_GMIN_DEFAULT),
		m_gC(NETLIST_GMIN_DEFAULT),
		m_V(0.0),
		m_state_on(0) { }


	ATTR_HOT void virtual update() override;

	nld_twoterm m_RB;
	nld_twoterm m_RC;

	// FIXME: this is needed so we have all terminals belong to one net list

	nld_twoterm m_BC_dummy;

protected:

	virtual void start() override;
	ATTR_HOT virtual void update_param() override;
	virtual void reset() override;
	NETLIB_UPDATE_TERMINALSI();

	nl_double m_gB; // base conductance / switch on
	nl_double m_gC; // collector conductance / switch on
	nl_double m_V; // internal voltage source
	UINT8 m_state_on;

private:
};

// -----------------------------------------------------------------------------
// nld_QBJT_EB
// -----------------------------------------------------------------------------


class NETLIB_NAME(QBJT_EB) : public NETLIB_NAME(QBJT)
{
public:
	ATTR_COLD NETLIB_NAME(QBJT_EB)(netlist_t &anetlist, const pstring &name)
	: NETLIB_NAME(QBJT)(BJT_EB, anetlist, name),
		m_D_CB(object_t::ANALOG, anetlist, "m_D_CB"),
		m_D_EB(object_t::ANALOG, anetlist, "m_D_EB"),
		m_D_EC(object_t::ANALOG, anetlist, "m_D_EC"),
		m_alpha_f(0),
		m_alpha_r(0)
		{ }

protected:

	virtual void start() override;
	virtual void reset() override;
	ATTR_HOT void update_param() override;
	ATTR_HOT void virtual update() override;
	NETLIB_UPDATE_TERMINALSI();

	generic_diode m_gD_BC;
	generic_diode m_gD_BE;

	nld_twoterm m_D_CB;  // gcc, gce - gcc, gec - gcc, gcc - gce | Ic
	nld_twoterm m_D_EB;  // gee, gec - gee, gce - gee, gee - gec | Ie
	nld_twoterm m_D_EC;  // 0, -gec, -gcc, 0 | 0

	nl_double m_alpha_f;
	nl_double m_alpha_r;

private:
};

NETLIB_NAMESPACE_DEVICES_END()

#endif /* NLD_BJT_H_ */
