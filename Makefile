# Root Makefile orchestrating build, run, results, plot
.PHONY: all build run results plot clean

SAMPLES ?= 5

all: build

build:
	$(MAKE) -C src

run: build
	./scripts/run_tests.sh $(SAMPLES)

results: run
	python3 scripts/collect_results.py

plot:
	python3 scripts/plot_case1.py
	python3 scripts/plot_case2.py
	python3 scripts/plot_case3.py
	python3 scripts/plot_all.py

TEX_OUT := report/lab1_report.tex
PDF_OUT := report/lab1_report.pdf

latex: $(TEX_OUT)

$(TEX_OUT): results plot
	python3 scripts/generate_report.py

pdf: $(PDF_OUT)

$(PDF_OUT): $(TEX_OUT)
	pdflatex -output-directory=report $(TEX_OUT)
	pdflatex -output-directory=report $(TEX_OUT)

clean:
	$(MAKE) -C src clean || true
	rm -rf bin
	rm -f data/case*_results.csv data/case*_summary.csv
	rm -f report/graphs/*.png
	rm -f $(TEX_OUT) $(PDF_OUT) report/*.aux report/*.log
