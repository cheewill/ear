BUILDDIR=out

debug: cmake_debug
	cd ${BUILDDIR}; make
.PHONY: debug

release: cmake_release
	cd ${BUILDDIR}; make
.PHONY: release

test: ${BUILDDIR}
	cd ${BUILDDIR}; cmake -DBUILD_TESTS=ON ..
	cd ${BUILDDIR}; make
	./${BUILDDIR}/test/main
.PHONY: test

cmake_debug: ${BUILDDIR}
	cd ${BUILDDIR}; cmake -DCMAKE_BUILD_TYPE=Debug ..
.PHONY: cmake_debug

cmake_release: ${BUILDDIR}
	cd ${BUILDDIR}; cmake -DCMAKE_BUILD_TYPE=Release ..
.PHONY: cmake_release

clean:
	rm -rf ${BUILDDIR}
.PHONY: clean

${BUILDDIR}:
	mkdir -p ${BUILDDIR}
