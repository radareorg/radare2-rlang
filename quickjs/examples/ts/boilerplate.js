

// boilerplate to glue r2 and quickjs
import('r2').then(entry).catch((e)=>{throw(e);});

function entry(r2) {
	try {
		require (['index'], function (foo) { foo.main(r2); });
	} catch (e) {
		r2.log("Error: " + e.message);
	}
}
