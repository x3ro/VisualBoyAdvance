#if !defined(AFX_GAMEOVERRIDES_H__EEEFE37F_F477_455D_8682_705FB2DBCC0C__INCLUDED_)
#define AFX_GAMEOVERRIDES_H__EEEFE37F_F477_455D_8682_705FB2DBCC0C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// GameOverrides.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// GameOverrides dialog

class GameOverrides : public CDialog
{
// Construction
public:
	GameOverrides(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(GameOverrides)
	enum { IDD = IDD_GAME_OVERRIDES };
	CEdit	m_name;
	CComboBox	m_flashSize;
	CComboBox	m_saveType;
	CComboBox	m_rtc;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(GameOverrides)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(GameOverrides)
	virtual void OnOK();
	afx_msg void OnDefaults();
	virtual void OnCancel();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GAMEOVERRIDES_H__EEEFE37F_F477_455D_8682_705FB2DBCC0C__INCLUDED_)
