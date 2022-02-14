# Vulnerabilities detected by polycruise (cross-language DIFA)
We evaluate polycruise on several popular open-source projects developed mainly in Python and C languages.
Eventually, 11 vulnerabilities in 5 projects below are validated to be exploitable, and corresponding PoCs are attached.

## [Bounter](https://github.com/RaRe-Technologies/bounter): [Vendor]RaRe-Technologies
#### [Vulnerability-1]: Null pointer reference
**Affected version**: version < 1.10 <br>
**Description**: With carefully constructed inputs (when the width of the hash bucket is set large enough), NULL pointer access could happen hence causing the Python to crash down. This allows attackers to conduct DoS attacks by inputing a huge width of hash bucket.<br>
**Exploitation**: PoC: [cms_increase_47.py](https://github.com/Daybreak2019/PolyCruise/edit/master/Experiments/PoC/bounter/vulnerability-1/cms_increase_47.py) and [Output](https://github.com/Daybreak2019/PolyCruise/edit/master/Experiments/PoC/bounter/vulnerability-1/output.txt).<br>
**CVE**: [CVE-2021-41497](https://nvd.nist.gov/vuln/detail/CVE-2021-41497)

## [Cvxopt](https://github.com/cvxopt/cvxopt): [Vendor]cvxopt.org
#### [Vulnerability-1]: Incomplete string comparison
**Affected version**: version <= 1.2.6 <br>
**Description**: Through carefully modify the name of a Capsule object, the PoC can easily bypass the validation in the **diag API** hence causing unexpected results (e.g., crash down during the execution of PoC). This allows attackers to conduct DoS attacks by construct fake Capsule objects. <br>
**Exploitation**: PoC: [diag_193.py](https://github.com/Daybreak2019/PolyCruise/tree/master/Experiments/PoC/cvxopt/vulnerability-1/diag_193.py) and [Output](https://github.com/Daybreak2019/PolyCruise/tree/master/Experiments/PoC/cvxopt/vulnerability-1/output.txt).<br>
**CVE**: [CVE-2021-41500](https://nvd.nist.gov/vuln/detail/CVE-2021-41500)
#### [Vulnerability-2]: Incomplete string comparison
**Affected version**: version <= 1.2.6 <br>
**Description**: Through carefully modify the name of a Capsule object, the PoC can easily bypass the validation in the **getfactor API** hence causing unexpected results (e.g., crash down during the execution of PoC). This allows attackers to conduct DoS attacks by construct fake Capsule objects. <br>
**Exploitation**: PoC: [getfactor_193.py](https://github.com/Daybreak2019/PolyCruise/tree/master/Experiments/PoC/cvxopt/vulnerability-2/getfactor_193.py) and [Output](https://github.com/Daybreak2019/PolyCruise/tree/master/Experiments/PoC/cvxopt/vulnerability-2/output.txt).<br>
**CVE**: [CVE-2021-41500](https://nvd.nist.gov/vuln/detail/CVE-2021-41500)
#### [Vulnerability-3]: Incomplete string comparison
**Affected version**: version <= 1.2.6 <br>
**Description**: Through carefully modify the name of a Capsule object, the PoC can easily bypass the validation in the **solve API** hence causing unexpected results (e.g., crash down during the execution of PoC). This allows attackers to conduct DoS attacks by construct fake Capsule objects. <br>
**Exploitation**: PoC: [solve_193.py](https://github.com/Daybreak2019/PolyCruise/tree/master/Experiments/PoC/cvxopt/vulnerability-3/solve_193.py) and [Output](https://github.com/Daybreak2019/PolyCruise/tree/master/Experiments/PoC/cvxopt/vulnerability-3/output.txt).<br>
**CVE**: [CVE-2021-41500](https://nvd.nist.gov/vuln/detail/CVE-2021-41500)
#### [Vulnerability-4]: Incomplete string comparison
**Affected version**: version <= 1.2.6 <br>
**Description**: Through carefully modify the name of a Capsule object, the PoC can easily bypass the validation in the **spsolve API** hence causing unexpected results (e.g., crash down during the execution of PoC). This allows attackers to conduct DoS attacks by construct fake Capsule objects. <br>
**Exploitation**: PoC: [spsolve_193.py](https://github.com/Daybreak2019/PolyCruise/tree/master/Experiments/PoC/cvxopt/vulnerability-4/spsolve_193.py) and [Output](https://github.com/Daybreak2019/PolyCruise/tree/master/Experiments/PoC/cvxopt/vulnerability-4/output.txt).<br>
**CVE**: [CVE-2021-41500](https://nvd.nist.gov/vuln/detail/CVE-2021-41500)

## [Japronto](https://github.com/squeaky-pl/japronto): [Vendor]squeaky_pl
#### [Vulnerability-1]: Unknown
**Affected version**: version < 0.1.1 <br>
**Description**: When passing byte stream context into the server API Response, the server runs abnormally, throws exceptions, and fails to deals with the following requests. This allows attackers to conduct DoS attacks. <br>
**Exploitation**: PoC: [feed_183_client.py](https://github.com/Daybreak2019/PolyCruise/tree/master/Experiments/PoC/japronto/vulnerability-1/feed_183_client.py) & [feed_183_server.py](https://github.com/Daybreak2019/PolyCruise/tree/master/Experiments/PoC/japronto/vulnerability-1/feed_183_server.py) and [Output](https://github.com/Daybreak2019/PolyCruise/tree/master/Experiments/PoC/japronto/vulnerability-1/output.txt).

## [Numpy](https://github.com/numpy/numpy): [Vendor]numpy.org
#### [Vulnerability-1]: Buffer overflow
**Affected version**: version < 1.19 <br>
**Description**: When loading a high-dimension array (larger than 32), a stack smashing error happens and causes the Python process to crash down. This allows attackers to conduct DoS attacks by carefully constructing a npy file.<br>
**Exploitation**: PoC: [array_nd_18939.py](https://github.com/Daybreak2019/PolyCruise/tree/master/Experiments/PoC/numpy/vulnerability-1/array_nd_18939.py) and [Output](https://github.com/Daybreak2019/PolyCruise/tree/master/Experiments/PoC/numpy/vulnerability-1/output.txt).<br>
**CVE**: [CVE-2021-33430](https://nvd.nist.gov/vuln/detail/CVE-2021-33430)
#### [Vulnerability-2]: Null pointer reference
**Affected version**: version < 1.19 <br>
**Description**: When constructing a loop to create high-dimension arrays repetitively, and keeping the references of the arrays effective, an error of NULL pointer access happens hence causing the Python to crash down. This allows attackers to conduct DoS attacks by repetitively creating and sort arrays.<br>
**Exploitation**: PoC: [array_nil_19038.py](https://github.com/Daybreak2019/PolyCruise/tree/master/Experiments/PoC/numpy/vulnerability-2/array_nil_19038.py) and [Output](https://github.com/Daybreak2019/PolyCruise/tree/master/Experiments/PoC/numpy/vulnerability-2/output.txt).<br>
**CVE**: [CVE-2021-41495](https://nvd.nist.gov/vuln/detail/CVE-2021-41495)
#### [Vulnerability-3]: Buffer overflow
**Affected version**: version < 1.19 <br>
**Description**: In the f2py module, when creating a high-dimension array through the **array API** (deliberately construct negative numbers in shape), an unexpected error occurs and causes Python to crash down. This allows attackers to conduct DoS attacks by carefully constructing an array with negative values in shape.<br>
**Exploitation**: PoC: [array_attr_19000.py](https://github.com/Daybreak2019/PolyCruise/tree/master/Experiments/PoC/numpy/vulnerability-3/array_attr_19000.py) and [Output](https://github.com/Daybreak2019/PolyCruise/tree/master/Experiments/PoC/numpy/vulnerability-3/output.txt).<br>
**CVE**: [CVE-2021-41496](https://nvd.nist.gov/vuln/detail/CVE-2021-41496)
#### [Vulnerability-4]: Incomplete string comparison
**Affected version**: version < 1.19 <br>
**Description**: In numpy.empty API, when checking the deprecated types with string comparison, no termintor is considered. [call-path]:numpy.empty(Python) -> PyArray_DescrAlignConverter -> _convert_from_any -> _convert_from_str -> strncmp. <br>
**Exploitation**: pending.<br>
**CVE**: [CVE-2021-34141](https://nvd.nist.gov/vuln/detail/CVE-2021-34141)
## [Pyo](https://github.com/belangeo/pyo)
#### [Vulnerability-1]: Buffer overflow
**Affected version**: version < < 1.03 <br>
**Description**: When using the Pyo library with audio type "jack", the server is initialized with an overlong (over 32) string, an error of buffer overflow happens and causes Python to crash down. This allows attackers to conduct DoS attacks by arbitrary constructing a overlong server name.<br>
**Exploitation**: PoC: [boot_221.py](https://github.com/Daybreak2019/PolyCruise/tree/master/Experiments/PoC/pyo/vulnerability-1/boot_221.py) and [Output](https://github.com/Daybreak2019/PolyCruise/tree/master/Experiments/PoC/pyo/vulnerability-1/output.txt).<br>
**CVE**: [CVE-2021-41498](https://nvd.nist.gov/vuln/detail/CVE-2021-41498)
#### [Vulnerability-2]: Buffer overflow
**Affected version**: version < < 1.03 <br>
**Description**: After initializing a Pyo server, an arbitrary file name (length > 256) is passed to the **recstart API** and an error of segment fault happens hence causing Python to crash down. This allows attackers to conduct DoS attacks by deliberately passing on an overlong audio file name.<br>
**Exploitation**: PoC: [restart_222.py](https://github.com/Daybreak2019/PolyCruise/tree/master/Experiments/PoC/pyo/vulnerability-2/restart_222.py) and [Output](https://github.com/Daybreak2019/PolyCruise/tree/master/Experiments/PoC/pyo/vulnerability-2/output.txt).<br>
**CVE**: [CVE-2021-41499](https://nvd.nist.gov/vuln/detail/CVE-2021-41499)
