package reactiveui;


public class ReactiveActivity_1
	extends reactiveui.ReactiveActivity
	implements
		mono.android.IGCUserPeer
{
	static final String __md_methods;
	static {
		__md_methods = 
			"";
		mono.android.Runtime.register ("ReactiveUI.ReactiveActivity`1, ReactiveUI, Version=6.4.0.1, Culture=neutral, PublicKeyToken=null", ReactiveActivity_1.class, __md_methods);
	}


	public ReactiveActivity_1 () throws java.lang.Throwable
	{
		super ();
		if (getClass () == ReactiveActivity_1.class)
			mono.android.TypeManager.Activate ("ReactiveUI.ReactiveActivity`1, ReactiveUI, Version=6.4.0.1, Culture=neutral, PublicKeyToken=null", "", this, new java.lang.Object[] {  });
	}

	java.util.ArrayList refList;
	public void monodroidAddReference (java.lang.Object obj)
	{
		if (refList == null)
			refList = new java.util.ArrayList ();
		refList.add (obj);
	}

	public void monodroidClearReferences ()
	{
		if (refList != null)
			refList.clear ();
	}
}
