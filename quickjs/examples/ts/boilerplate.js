// boilerplate to glue r2 and quickjs
var Module = [];
var System = {
	register: function (name, unk, func) {
		  Module[name] = func;
	}
}
import('r2').then(entry).catch((e)=>{throw(e);});
function entry(r2) {
	try {
		r2.cmdj = function(x) {
			return JSON.parse(r2.cmd(x));
		}
		const main = Module.getMain();
		if (main) {
			main ("");
		}
		Module["index"].main(r2); 
	} catch (e) {
		r2.log("Error: " + e.message);
	}
}
