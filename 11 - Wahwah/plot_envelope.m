atc = 0.75;
decay_time = .1;

[sig, fs, nbits] = wavread('sgt_pepper_excerpt.wav');
t = (1:length(sig))/fs;

clf
plot(t, sig);
xlim([0 length(sig)/fs]);
ylim([-0.8 0.8]);
xlabel('Time (seconds)', 'FontSize', 13);
ylabel('Amplitude', 'FontSize', 13);
hold

%atc = attack_time^(1/fs);
dtc = decay_time^(1/fs);

env = zeros(length(sig), 1);
for i = 2:length(sig)
    samp = abs(sig(i));
    if samp > env(i-1)
       env(i) = env(i-1) + (samp - env(i-1))*(1 - atc);
    else
       env(i) = env(i-1) * dtc;
    end
end

plot(t(2:length(t)), env(1:(length(env)-1)), 'k', 'LineWidth', 2);

set(gcf, 'PaperUnits', 'inches');
set(gcf, 'PaperSize', [8 3]);
set(gcf, 'PaperPosition', [0 0 8 3]);
print -depsc envelope.eps